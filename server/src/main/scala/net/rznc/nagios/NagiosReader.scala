package net.rznc.nagios

import akka.actor.Actor
import akka.event.Logging
import scala.concurrent.Future
import scala.concurrent.duration._
import scala.util.{ Failure, Success }
import spray.client.pipelining._
import spray.http._
import spray.httpx.unmarshalling.Unmarshaller
import spray.http.MediaTypes._
import spray.http.HttpRequest

class NagiosReader extends Actor with NagiosParser {

  import context.dispatcher

  case object Poll

  val STATUS_URL = "/cgi-bin/status.cgi?host=all&servicestatustypes=28"

  val log = Logging(context.system, this)
  val config = context.system.settings.config
  val statusUrl = config.getString("nagios-monitor.nagios.url").stripSuffix("/") + STATUS_URL
  val username = config.getString("nagios-monitor.nagios.username")
  val password = config.getString("nagios-monitor.nagios.password")

  implicit val StatusUnmarshaller = Unmarshaller[NagiosStatus](`text/html`) {
    case HttpEntity.NonEmpty(contentType, data) => parse(data.toByteArray)
  }

  val pipeline: HttpRequest => Future[NagiosStatus] = (
    addCredentials(BasicHttpCredentials(username, password))
    ~> sendReceive
    ~> unmarshal[NagiosStatus]
  )

  context.system.scheduler.schedule(1.second, 10.seconds, self, Poll)

  def compareStatus(prevStatus: NagiosStatus, prevUpdate: StatusUpdate, newStatus: NagiosStatus): StatusUpdate = {
    val (pc, pw) = (prevStatus.critical, prevStatus.warning)
    val (nc, nw) = (newStatus.critical, newStatus.warning)

    val criticalIncr = if ((nc -- pc).nonEmpty) 1 else 0
    val warningIncr = if ((nw -- pc -- pw).nonEmpty) 1 else 0
    val okIncr = if ((pc ++ pw -- nc -- nw).nonEmpty) 1 else 0

    StatusUpdate(
      newStatus.critical.size,
      prevUpdate.criticalSerial + criticalIncr,
      newStatus.warning.size,
      prevUpdate.warningSerial + warningIncr,
      newStatus.okCount,
      prevUpdate.okSerial + okIncr
    )
  }

  def receive = receiveStatus(NagiosStatus(), StatusUpdate())

  def receiveStatus(prevStatus: NagiosStatus, prevUpdate: StatusUpdate): Receive = {
    case Poll =>
      pipeline {
        Get(statusUrl)
      } onComplete {
        case Success(newStatus: NagiosStatus) =>
          val newUpdate = compareStatus(prevStatus, prevUpdate, newStatus)
          log.debug("Status update: {}", newUpdate)
          context.parent ! newUpdate
          context become receiveStatus(newStatus, newUpdate)
        case Success(unexpected) =>
          log.error("Unexpected response: {}", unexpected)
        case Failure(error) =>
          log.warning("Error fetching status: {}", error)
      }
  }

}
