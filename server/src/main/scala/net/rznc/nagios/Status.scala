package net.rznc.nagios

import akka.actor._

import Commands._
import NagiosReader.NagiosException
import NagiosParser.NagiosStatus

object Status {

  case class Register(listener: ActorRef)

  def props(): Props = Props(new Status())

}

class Status extends Actor with ActorLogging {

  var listeners = IndexedSeq.empty[ActorRef]

  override def preStart() =
    context.actorOf(Props[NagiosReader], "reader")

  def receive = receive(NagiosStatus())

  def receive(prevStatus: NagiosStatus): Receive = {
    case newStatus: NagiosStatus =>
      val newUpdate = updateStatus(newStatus, prevStatus)
      sendStatus(newUpdate)
      context become receive(newStatus)

    case _: NagiosException =>
      sendStatus(Message("Nagios Error", RED))

    case Status.Register(listener) =>
      listeners :+= listener

    case Terminated(listener) =>
      listeners = listeners.filterNot(_ == listener)
  }

  def sendStatus(message: AnyRef) = listeners.foreach(_ ! message)

  def updateStatus(newStatus: NagiosStatus, prevStatus: NagiosStatus) =
    (newStatus, prevStatus) match {
      case (NagiosStatus(nc, nw, no), NagiosStatus(pc, pw, _)) =>
        //val newCounts = Counts(nc.size, nw.size, no)
        /*
        val newSerials = Serials(
          if ((nc -- pc).nonEmpty) sc + 1 else sc,
          if ((nw -- pc -- pw).nonEmpty) sw + 1 else sw,
          if ((pc ++ pw -- nc -- nw).nonEmpty) so + 1 else so
        )
        */
        val newUpdate = Update(nc.size, nw.size, no)

        newUpdate
    }

}
