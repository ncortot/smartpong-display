package net.rznc.nagios

import akka.actor._
import com.typesafe.config.ConfigFactory

object Main extends App {

  val config = ConfigFactory.load().getConfig("nagios-monitor")
  val system = ActorSystem("nagios-monitor", config)

  system.actorOf(Props[Status], "status")

  val serialPort = config.getString("serial.port")
  if (serialPort.nonEmpty)
    system.actorOf(SerialHandler.props(serialPort), "serial")

  val serverAddress = config.getString("server.address")
  val serverPort = config.getInt("server.port")
  if (serverAddress.nonEmpty && serverPort != 0)
    system.actorOf(TCPServer.props(serverAddress, serverPort), "server")

}
