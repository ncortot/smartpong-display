package net.rznc.nagios

import akka.actor.{ ActorSystem, Props }


object Main extends App {

  ActorSystem("nagios-monitor").actorOf(Props[Server])

}
