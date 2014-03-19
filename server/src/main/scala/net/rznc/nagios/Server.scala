package net.rznc.nagios

import java.net.InetSocketAddress

import akka.actor._
import akka.event.Logging
import akka.io._
import akka.io.Tcp._

class Server extends Actor {

  import context.system

  val log = Logging(context.system, this)
  var clients = Set.empty[ActorRef]

  val config = context.system.settings.config
  val address = config.getString("nagios-monitor.server.address")
  val port = config.getInt("nagios-monitor.server.port")
  val secret = config.getString("nagios-monitor.server.secret")

  IO(Tcp) ! Bind(self, new InetSocketAddress(address, port))
  context.actorOf(Props[NagiosReader], "poller")

  def receive = {

    case Bound(localAddress) =>
      log.info(s"Listening on $localAddress")

    case CommandFailed(bind: Bind) =>
      log.error(s"Bind failed on ${bind.localAddress}")
      context stop self

    case Connected(remote, local) =>
      val handler = context.actorOf(Props(classOf[ClientHandler], secret))
      log.info(s"ClientHandler $handler started for $remote")
      context.watch(handler)
      clients += handler
      sender() ! Register(handler)

    case Terminated(handler) =>
      log.info(s"ClientHandler $handler terminated")
      context.unwatch(handler)
      clients -= handler

    case status: StatusUpdate =>
      clients foreach (_ ! status)

  }

}
