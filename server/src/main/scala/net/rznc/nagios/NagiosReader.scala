package net.rznc.nagios

import akka.actor._
import scala.concurrent.Future
import scala.concurrent.duration._
import scala.util.{ Failure, Success }
import spray.client.pipelining._
import spray.http._
import spray.httpx.unmarshalling.Unmarshaller
import spray.http.MediaTypes._
import spray.http.HttpRequest

object NagiosReader {

  case object Poll

  sealed class NagiosException(message: String)

  case object NagiosError extends NagiosException("Error reading Nagios status.")
  case object NagiosTimeout extends NagiosException("Timeout reading Nagios status.")

  val STATUS_URL = "/cgi-bin/status.cgi?host=all&servicestatustypes=28"

}

class NagiosReader extends Actor with ActorLogging {

  import context.dispatcher
  import NagiosParser._
  import NagiosReader._

  var poll: Option[Cancellable] = None

  val config = context.system.settings.config
  val statusUrl = config.getString("nagios.url").stripSuffix("/") + STATUS_URL
  val username = config.getString("nagios.username")
  val password = config.getString("nagios.password")

  implicit val StatusUnmarshaller = Unmarshaller[NagiosStatus](`text/html`) {
    case HttpEntity.NonEmpty(contentType, data) =>
      parse(data.toByteArray)
  }

  val pipeline: HttpRequest => Future[NagiosStatus] = (
    addCredentials(BasicHttpCredentials(username, password))
    ~> sendReceive
    ~> unmarshal[NagiosStatus]
  )

  override def preStart() =
    poll = Some(context.system.scheduler.schedule(1.second, 2.seconds, self, Poll))

  override def postStop() =
    poll.map(_.cancel())

  def receive = receiveIdle(0)

  def receiveIdle(lastSerial: Int): Receive = {
    case Poll =>
      val serial = lastSerial + 1
      pipeline(Get(statusUrl)) onComplete {
        case Success(status: NagiosStatus) => self ! (serial, status)
        case failure @ Failure(error) => self ! (serial, failure)
      }
      context become receivePending(serial)
  }

  def receivePending(pendingSerial: Int): Receive = {
    case Poll =>
      log.warning("Nagios status request {} timed out.", pendingSerial)
      context.parent ! NagiosTimeout
      context become receiveIdle(pendingSerial)
      self ! Poll

    case (serial: Int, status: NagiosStatus) if serial == pendingSerial =>
      log.debug("Received status for request {}.", serial)
      context.parent ! status
      context become receiveIdle(pendingSerial)

    case (serial: Int, Failure(error)) if serial == pendingSerial =>
      log.warning("Error fetching status request {}: {}", serial, error)
      context.parent ! NagiosError
      context become receiveIdle(pendingSerial)

    case (serial: Int, status: NagiosStatus) =>
      log.warning("Received status for timed-out request {}.", serial)

    case (serial: Int, Failure(error)) =>
      log.warning("Error fetching timed-out status request {}: {}", serial, error)
  }

}
