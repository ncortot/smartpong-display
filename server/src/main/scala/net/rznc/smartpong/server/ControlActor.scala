package net.rznc.smartpong.server

import akka.actor._

import ControlActor._

object ControlActor {

  def props(displayActor: ActorRef): Props = Props(new ControlActor(displayActor))

  sealed trait Action
  case class Command(data: String) extends Action

  sealed trait State
  case object DefaultState extends State

  case object Disconnected

}

class ControlActor(displayActor: ActorRef) extends Actor with ActorLogging with FSM[State, Score] {

  implicit val ec = context.dispatcher

  startWith(DefaultState, Score())

  when(DefaultState) {
    case Event(command: Command, score) =>
      val newScore = update(command, score)
      sender() ! newScore
      displayActor ! newScore
      stay() using newScore
    case Event(ControlActor.Disconnected, _) =>
      log.debug(s"Client ${sender()} disconnected")
      stay()
    case Event(message, _) =>
      log.warning(s"Unexpected message: $message")
      stay()
  }

  private def update(command: Command, s: Score): Score = command match {
    case Command("noop") =>
      s
    case Command("p1+") =>
      s.copy(p1 = s.p1 + 1)
    case Command("p1-") =>
      s.copy(p1 = math.max(0, s.p1 - 1))
    case Command("p2+") =>
      s.copy(p2 = s.p2 + 1)
    case Command("p2-") =>
      s.copy(p2 = math.max(0, s.p2 - 1))
    case Command("s1+") =>
      s.copy(s1 = s.s1 + 1)
    case Command("s1-") =>
      s.copy(s1 = math.max(0, s.s1 - 1))
    case Command("s2+") =>
      s.copy(s2 = s.s2 + 1)
    case Command("s2-") =>
      s.copy(s2 = math.max(0, s.s2 - 1))
    case Command("swap") =>
      s.copy(p1 = s.p2, p2 = s.p1, s1 = s.s2, s2 = s.s1)
    case Command("srv1") =>
      s.copy(service = Score.Player1)
    case Command("srv2") =>
      s.copy(service = Score.Player2)
    case Command("reset") =>
      Score()
  }

}
