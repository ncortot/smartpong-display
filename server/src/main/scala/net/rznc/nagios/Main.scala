package net.rznc.nagios

import akka.actor._
import com.typesafe.config.ConfigFactory

object Main extends App {

  val config = ConfigFactory.load()
  val system = ActorSystem("nagios-monitor", config.getConfig("nagios-monitor").withFallback(config))
  system.actorOf(Props[Server], "server")
  system.actorOf(Props[Status], "status")

}
