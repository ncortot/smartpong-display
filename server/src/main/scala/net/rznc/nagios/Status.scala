package net.rznc.nagios

import akka.actor._
import Messages._
import NagiosReader.NagiosException
import NagiosParser.NagiosStatus

class Status extends Actor with ActorLogging {

  val server = context.system.actorSelection("/user/server")

  override def preStart() =
    context.actorOf(Props[NagiosReader], "reader")

  def receive = receiveStatus(NagiosStatus(), StatusUpdate())

  def receiveStatus(prevStatus: NagiosStatus, prevUpdate: StatusUpdate): Receive = {
    case newStatus: NagiosStatus =>
      val newUpdate = updateStatus(newStatus, prevStatus, prevUpdate)
      server ! newUpdate
      context become receiveStatus(newStatus, newUpdate)

    case _: NagiosException =>
      server ! StatusError()
  }

  def updateStatus(newStatus: NagiosStatus, prevStatus: NagiosStatus, prevUpdate: StatusUpdate) =
    (newStatus, prevStatus, prevUpdate) match {
      case (NagiosStatus(nc, nw, no), NagiosStatus(pc, pw, _), StatusUpdate(_, Serials(sc, sw, so))) =>
        val newCounts = Counts(nc.size, nw.size, no)
        val newSerials = Serials(
          if ((nc -- pc).nonEmpty) sc + 1 else sc,
          if ((nw -- pc -- pw).nonEmpty) sw + 1 else sw,
          if ((pc ++ pw -- nc -- nw).nonEmpty) so + 1 else so
        )
        val newUpdate = StatusUpdate(newCounts, newSerials)
        if (newUpdate == prevUpdate)
          log.debug("Status unchanged: {}, {}", newCounts, newSerials)
        else
          log.debug("Status changed: {}, {}", newCounts, newSerials)
        newUpdate
    }

}
