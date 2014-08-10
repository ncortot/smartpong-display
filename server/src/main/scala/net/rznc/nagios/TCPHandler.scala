package net.rznc.nagios

import akka.actor._
import akka.util._
import akka.io.Tcp._

import Commands.StatusMessage

object TCPHandler {

  def props(connection: ActorRef): Props = Props(new TCPHandler(connection))

}

class TCPHandler(connection: ActorRef) extends Actor with ActorLogging {

  context watch connection

  lazy val status = context.system.actorSelection("/user/status")

  def authenticate(token: String): Unit = {
    val secret = context.system.settings.config.getString("server.secret")
    if (token == secret) {
      log.info("Client authenticated")
      status ! Status.Register(self)
      context become authenticated
    } else {
      log.warning("Authentication failed")
      connection ! Close
      context become shutdown
    }
  }

  def authenticated: Receive = ({
    case message: StatusMessage =>
      connection ! Write(ByteString(message.command.getBytes))
    case Received(data) =>
      log.info("Message received: {}", data.utf8String.stripLineEnd)
  }: Receive) orElse shutdown

  def receive = ({
    case Received(data) =>
      authenticate(data.utf8String.stripLineEnd)
  }: Receive) orElse shutdown

  def shutdown: Receive = {
    case PeerClosed =>
      log.info("Connection closed by peer")
      context unwatch connection
      context stop self
    case Terminated =>
      log.warning("Connection terminated")
      context unwatch connection
      context stop self
  }

}
