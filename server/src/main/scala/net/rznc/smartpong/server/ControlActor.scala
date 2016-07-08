package net.rznc.smartpong.server

import akka.actor.{Actor, ActorLogging, ActorRef, Props}
import akka.agent.Agent

object ControlActor {

  def props(refreshActor: ActorRef, scoreAgent: Agent[Score]): Props =
    Props(new ControlActor(refreshActor, scoreAgent))

  sealed trait Action
  case class Command(data: String) extends Action

  case object Disconnected

}

class ControlActor(refreshActor: ActorRef, scoreAgent: Agent[Score])
  extends Actor with ActorLogging {

  implicit val ec = context.dispatcher

  def receive = {
    case "p1+" =>
      update(sender()) { s => s.copy(p1 = s.p1 + 1) }
    case "p1-" =>
      update(sender()) { s => s.copy(p1 = math.max(0, s.p1 - 1)) }
    case "p2+" =>
      update(sender()) { s => s.copy(p2 = s.p2 + 1) }
    case "p2-" =>
      update(sender()) { s => s.copy(p2 = math.max(0, s.p2 - 1)) }
    case "s1+" =>
      update(sender()) { s => s.copy(s1 = s.s1 + 1) }
    case "s1-" =>
      update(sender()) { s => s.copy(s1 = math.max(0, s.s1 - 1)) }
    case "s2+" =>
      update(sender()) { s => s.copy(s2 = s.s2 + 1) }
    case "s2-" =>
      update(sender()) { s => s.copy(s2 = math.max(0, s.s2 - 1)) }
    case "swap" =>
      update(sender()) { s => s.copy(p1 = s.p2, p2 = s.p1, s1 = s.s2, s2 = s.s1) }
    case "srv1" =>
      update(sender()) { s => s.copy(initialService = 1) }
    case "srv2" =>
      update(sender()) { s => s.copy(initialService = 2) }
    case "reset" =>
      update(sender()) { s => Score() }
    case ControlActor.Disconnected =>
      log.debug(s"Client ${sender()} disconnected")
    case message: Any =>
      log.warning(s"Unexpected message: $message")
  }

  private def update(ref: ActorRef)(f: Score => Score): Unit =
    scoreAgent.alter(f).onComplete {
      case _: Any =>
        ref ! scoreAgent()
        refreshActor ! RefreshActor.Refresh
    }

}
