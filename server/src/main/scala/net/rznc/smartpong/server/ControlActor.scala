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

  private def currentService(s: Score): Score.Service =
    if (s.p1 >= 10 && s.p2 >= 10)
      if ((s.p1 + s.p2) % 2 > 0) otherService(s.initial) else s.initial
    else
      if ((s.p1 + s.p2) % 4 > 1) otherService(s.initial) else s.initial

  private def otherService: Score.Service => Score.Service = {
    case Score.Player1 => Score.Player2
    case Score.Player2 => Score.Player1
    case Score.Undefined => Score.Undefined
  }

  private def updateService(s: Score): Score = s.copy(service = currentService(s))

  private def p1won(s: Score) = s.p1 >= 11 && s.p1 - s.p2 >= 2

  private def p2won(s: Score) = s.p2 >= 11 && s.p2 - s.p1 >= 2

  private def addPoint(s: Score): Score = updateService(
    if (p1won(s))
      s.copy(s1 = s.s1 + 1, completed = true)
    else if (p2won(s))
      s.copy(s2 = s.s2 + 1, completed = true)
    else
      s
  )

  private def subPoint(s: Score): Score = updateService(
    if (p1won(stateData))
      s.copy(s1 = math.max(0, s.s1 - 1), completed = false)
    else if (p2won(stateData))
      s.copy(s2 = math.max(0, s.s2 - 1), completed = false)
    else
      s
  )

  private def update(command: Command, s: Score): Score = command match {
    case Command("p1+") if s.initial != Score.Undefined && !s.completed =>
      addPoint(s.copy(p1 = s.p1 + 1))
    case Command("p1-") if s.p1 > 0 =>
      subPoint(s.copy(p1 = s.p1 - 1))
    case Command("p2+") if s.initial != Score.Undefined && !s.completed =>
      addPoint(s.copy(p2 = s.p2 + 1))
    case Command("p2-") if s.p2 > 0 =>
      subPoint(s.copy(p2 = s.p2 - 1))
    case Command("reset") =>
      Score()
    case Command("s1+") =>
      s.copy(s1 = s.s1 + 1)
    case Command("s1-") if s.s1 > 0 =>
      s.copy(s1 = s.s1 - 1)
    case Command("s2+") =>
      s.copy(s2 = s.s2 + 1)
    case Command("s2-") if s.s2 > 0 =>
      s.copy(s2 = s.s2 - 1)
    case Command("srv1") if s.p1 == 0 && s.p2 == 0 =>
      s.copy(initial = Score.Player1, service = Score.Player1)
    case Command("srv2") if s.p1 == 0 && s.p2 == 0 =>
      s.copy(initial = Score.Player2, service = Score.Player2)
    case Command("swap") if s.completed =>
      s.copy(p1 = 0, p2 = 0, s1 = s.s2, s2 = s.s1, service = s.initial, completed = false)
    case Command("noop" | "p1+" | "p1-" | "p2+" | "p2-" | "s1-"  | "s2-" | "srv1" | "srv2" | "swap") =>
      s
  }

}
