package net.rznc.smartpong.server

import akka.actor.{Actor, ActorLogging, ActorRef, Props}
import akka.agent.Agent

import ControlActor._

object ControlActor {

  def props(displayActor: ActorRef): Props = Props(new ControlActor(displayActor))

  sealed trait Action
  case class Command(data: String) extends Action

  case object Disconnected

}

class ControlActor(displayActor: ActorRef) extends Actor with ActorLogging {

  implicit val ec = context.dispatcher

  def receive = {
    case Command("p1+") =>
      update(sender()) { s => s.copy(p1 = s.p1 + 1) }
    case Command("p1-") =>
      update(sender()) { s => s.copy(p1 = math.max(0, s.p1 - 1)) }
    case Command("p2+") =>
      update(sender()) { s => s.copy(p2 = s.p2 + 1) }
    case Command("p2-") =>
      update(sender()) { s => s.copy(p2 = math.max(0, s.p2 - 1)) }
    case Command("s1+") =>
      update(sender()) { s => s.copy(s1 = s.s1 + 1) }
    case Command("s1-") =>
      update(sender()) { s => s.copy(s1 = math.max(0, s.s1 - 1)) }
    case Command("s2+") =>
      update(sender()) { s => s.copy(s2 = s.s2 + 1) }
    case Command("s2-") =>
      update(sender()) { s => s.copy(s2 = math.max(0, s.s2 - 1)) }
    case Command("swap") =>
      update(sender()) { s => s.copy(p1 = s.p2, p2 = s.p1, s1 = s.s2, s2 = s.s1) }
    case Command("srv1") =>
      update(sender()) { s => s.copy(service = Score.Player1) }
    case Command("srv2") =>
      update(sender()) { s => s.copy(service = Score.Player2) }
    case Command("reset") =>
      update(sender()) { s => Score() }
    case ControlActor.Disconnected =>
      log.debug(s"Client ${sender()} disconnected")
    case message: Any =>
      log.warning(s"Unexpected message: $message")
  }

  private def update(ref: ActorRef)(f: Score => Score): Unit = {
    val newScore = f(Score())
    ref ! newScore
    displayActor ! newScore
  }

}
