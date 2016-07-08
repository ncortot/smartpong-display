package net.rznc.smartpong.server

import akka.actor.{Actor, ActorLogging, ActorRef, FSM, Props, Terminated}
import scala.concurrent.duration._

import RefreshActor._

object RefreshActor {

  def props(): Props = Props(new RefreshActor())

  sealed trait State
  case object DefaultState extends State

  case object Refresh
  case object Tick

}

class RefreshActor extends Actor with ActorLogging with FSM[State, List[ActorRef]] {

  implicit val ec = context.dispatcher

  context.system.scheduler.schedule(5.seconds, 10.seconds, self, Tick)

  startWith(DefaultState, Nil)

  when(DefaultState) {
    case Event(Tick, clients) =>
      self ! Refresh
      stay()
    case Event(ref: ActorRef, clients) =>
      ref ! Score()
      context watch ref
      stay() using ref :: clients
    case Event(Refresh, clients) =>
      clients foreach { _ ! Score() }
      stay()
    case Event(Terminated(ref), clients) =>
      context unwatch ref
      stay() using clients.filter(_ != ref)
    case Event(message, _) =>
      log.warning(s"Unexpected message: $message")
      stay()
  }

}
