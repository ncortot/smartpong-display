package net.rznc.smartpong.server

import akka.actor._
import scala.concurrent.duration._

import DisplayActor._

object DisplayActor {

  def props(): Props = Props(new DisplayActor())

  sealed trait State
  case object DefaultState extends State

  case object Update

}

class DisplayActor extends Actor with ActorLogging with FSM[State, (Score, List[ActorRef])] {

  implicit val ec = context.dispatcher

  context.system.scheduler.schedule(5.seconds, 30.seconds, self, Update)

  startWith(DefaultState, (Score(), Nil))

  when(DefaultState) {
    case Event(Update, (score, clients)) =>
      self ! score
      stay()
    case Event(ref: ActorRef, (score, clients)) =>
      ref ! score
      context watch ref
      stay() using ((score, ref :: clients))
    case Event(score: Score, (_, clients)) =>
      clients foreach { _ ! score }
      stay() using ((score, clients))
    case Event(Terminated(ref), (score, clients)) =>
      context unwatch ref
      stay() using ((score, clients.filter(_ != ref)))
    case Event(message, _) =>
      log.warning(s"Unexpected message: $message")
      stay()
  }

}
