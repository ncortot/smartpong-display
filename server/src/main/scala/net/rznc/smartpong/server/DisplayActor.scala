package net.rznc.smartpong.server

import akka.actor.{Actor, ActorLogging, ActorRef, Props}
import akka.agent.Agent
import akka.stream.actor.ActorPublisher
import akka.stream.actor.ActorPublisherMessage.{Cancel, Request}

object DisplayActor {

  def props(refreshActor: ActorRef, scoreAgent: Agent[Score]): Props =
    Props(new DisplayActor(refreshActor, scoreAgent))

}

class DisplayActor(refreshActor: ActorRef, scoreAgent: Agent[Score])
  extends Actor with ActorLogging with ActorPublisher[Score] {

  override def preStart(): Unit = {
    log.debug("Starting display client actor")
    refreshActor ! self
  }

  def receive = {
    case Cancel =>
      log.debug("Display client closed connexion")
      context stop self
    case Request(_) =>
      if (isActive && totalDemand > 0)
        onNext(scoreAgent())
    case RefreshActor.Refresh =>
      if (isActive && totalDemand > 0)
        onNext(scoreAgent())
    case message: Any =>
      log.warning(s"Unexpected message: $message")
  }

}
