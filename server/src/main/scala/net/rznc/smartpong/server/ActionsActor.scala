package net.rznc.smartpong.server

import akka.actor.{Actor, ActorLogging, Props}

import scala.collection.mutable
import ActionsActor._

object ActionsActor {

  case class Action(action: String)
  case object GetActions

  def props(): Props = Props(new ActionsActor())

}

class ActionsActor extends Actor with ActorLogging {

  private val actions = mutable.Queue[String]()

  def receive = {
    case Action(action) =>
      log.debug(s"Got action: $action")
      actions += action
    case GetActions =>
      log.debug("Flushed all actions")
      sender ! actions.dequeueAll(_ => true)
  }

}
