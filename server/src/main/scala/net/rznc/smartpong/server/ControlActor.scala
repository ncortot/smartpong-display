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

  private def p1won(s: Score) = s.p1 >= 11 && s.p1 - s.p2 >= 2

  private def p2won(s: Score) = s.p2 >= 11 && s.p2 - s.p1 >= 2

  private def swapService(s: Score): Score = s.service match {
    case Score.Undefined => s
    case Score.Player1 => s.copy(service = Score.Player2)
    case Score.Player2 => s.copy(service = Score.Player1)
  }

  private def addPoint(s: Score): Score = addWon(addService(s))

  private def addService(s: Score): Score =
    if ((s.p1 >= 10 && s.p2 >= 10) || ((s.p1 + s.p2) % 2 == 0))
      swapService(s)
    else
      s

  private def addWon(s: Score): Score =
    if (p1won(s))
      s.copy(s1 = s.s1 + 1, completed = true)
    else if (p2won(s))
      s.copy(s2 = s.s2 + 1, completed = true)
    else
      s

  private def subPoint(s: Score): Score = subWon(subService(s))

  private def subService(s: Score): Score =
    if ((s.p1 >= 10 && s.p2 >= 10) || ((s.p1 + s.p2 % 2) == 0))
      swapService(s)
    else
      s

  private def subWon(s: Score): Score =
    if (p1won(stateData))
      s.copy(s1 = math.max(0, s.s1 - 1), completed = false)
    else if (p2won(stateData))
      s.copy(s2 = math.max(0, s.s2 - 1), completed = false)
    else
      s

  private def update(command: Command, s: Score): Score = command match {
    case Command("noop") =>
      s
    case Command("p1+") =>
      if (s.completed) s else addPoint(s.copy(p1 = s.p1 + 1))
    case Command("p1-") =>
      if (s.p1 <= 0) s else subPoint(s.copy(p1 = s.p1 - 1))
    case Command("p2+") =>
      if (s.completed) s else addPoint(s.copy(p2 = s.p2 + 1))
    case Command("p2-") =>
      if (s.p2 <= 0) s else subPoint(s.copy(p2 = s.p2 - 1))
    case Command("s1+") =>
      s.copy(s1 = s.s1 + 1)
    case Command("s1-") =>
      s.copy(s1 = math.max(0, s.s1 - 1))
    case Command("s2+") =>
      s.copy(s2 = s.s2 + 1)
    case Command("s2-") =>
      s.copy(s2 = math.max(0, s.s2 - 1))
    case Command("swap") =>
      if (!s.completed) s else s.copy(p1 = 0, p2 = 0, s1 = s.s2, s2 = s.s1, completed = false)
    case Command("srv1") =>
      s.copy(service = Score.Player1)
    case Command("srv2") =>
      s.copy(service = Score.Player2)
    case Command("reset") =>
      Score()
  }

}
