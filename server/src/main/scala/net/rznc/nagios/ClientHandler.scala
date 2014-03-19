package net.rznc.nagios

import akka.actor.{ Actor, ActorRef }
import akka.io.Tcp._
import akka.event.Logging

class ClientHandler(secret: String) extends Actor {

  val SECRET_MAX = 1024

  val log = Logging(context.system, this)

  def authenticate(token: String): Unit = {
    if (token == secret) {
      log.info("Client {} authenticated", sender())
      context become authenticated(sender())
    } else {
      log.warning("Authentication error from {}", sender())
    }
  }

  def authenticated(connection: ActorRef): Receive = {
    case status: StatusUpdate => connection ! Write(status.toByteString)
    case PeerClosed => context stop self
  }

  def receive = {
    case Received(data) => authenticate(data.take(SECRET_MAX).utf8String.stripLineEnd)
    case PeerClosed => context stop self
  }

}
