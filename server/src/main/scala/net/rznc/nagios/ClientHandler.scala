package net.rznc.nagios

import akka.actor._
import akka.io.Tcp._

object ClientHandler {

  def props(connection: ActorRef): Props = Props(new ClientHandler(connection))

}

class ClientHandler(connection: ActorRef) extends Actor with ActorLogging {

  context watch connection

  def authenticate(token: String): Unit = {
    val secret = context.system.settings.config.getString("server.secret")
    if (token == secret) {
      log.info("Client authenticated")
      context become authenticated
    } else {
      log.warning("Authentication failed")
      context unwatch connection
      context stop self
    }
  }

  def authenticated: Receive = ({
    case write: Write =>
      log.debug("Sending message")
      connection ! write
    case Received(data) =>
      log.info("Message received: {}", data.utf8String.stripLineEnd)
  }: Receive) orElse shutdown

  def receive = ({
    case _: Write =>
      log.info("Client is not authenticated")
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
