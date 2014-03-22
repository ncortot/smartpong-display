package net.rznc.nagios

import java.net.InetSocketAddress

import akka.actor._
import akka.io._
import akka.io.Tcp._
import Messages._

class Server extends Actor with ActorLogging {

  import context.system

  var socket: Option[ActorRef] = None

  override def preStart() = {
    val config = context.system.settings.config
    val address = config.getString("server.address")
    val port = config.getInt("server.port")
    IO(Tcp) ! Bind(self, new InetSocketAddress(address, port))
  }

  override def postStop() =
    socket map (_ ! Unbind)

  def receive = {
    case Bound(localAddress) =>
      log.info(s"Listening on ${localAddress.getHostName}:${localAddress.getPort}")
      socket = Some(sender())

    case CommandFailed(Bind(_, localAddress, _, _, _)) =>
      log.error(s"Bind failed on ${localAddress.getHostName}:${localAddress.getPort}")
      context stop self

    case Connected(remote, local) =>
      val connection = sender()
      val name = s"${remote.getHostName}:${remote.getPort}"
      val handler = context.actorOf(ClientHandler.props(connection), name)
      log.info(s"Client $name connected")
      connection ! Register(handler)

    case message: StatusMessage =>
      val write = Write(message.toByteString)
      context.children foreach (_ ! write)
  }

}
