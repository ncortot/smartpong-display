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
      parseStatus(newStatus, prevStatus)
      context become receive(newStatus)

    case _: NagiosException =>
      sendStatus(Message("Nagios Error", RED))

    case Status.Register(listener) =>
      listeners :+= listener

    case Terminated(listener) =>
      listeners = listeners.filterNot(_ == listener)
  }

  def parseStatus(newStatus: NagiosStatus, prevStatus: NagiosStatus): Unit =
    (newStatus, prevStatus) match {
      case (NagiosStatus(nc, nw, no), NagiosStatus(pc, pw, _)) =>
        sendStatus(Update(nc.size, nw.size, no))
        if ((nc -- pc).nonEmpty)
          sendStatus(Notification(CRITICAL))
        else if ((nw -- pc -- pw).nonEmpty)
          sendStatus(Notification(WARNING))
        else if ((pc ++ pw -- nc -- nw).nonEmpty)
          sendStatus(Notification(OK))
    }

  def sendStatus(message: AnyRef) = listeners.foreach(_ ! message)

}
