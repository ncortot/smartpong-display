package net.rznc.nagios

import akka.actor._
import java.io._

import Commands.StatusMessage

object SerialHandler {

  def props(port: String): Props = Props(new SerialHandler(port))
  
}

class SerialHandler(port: String) extends Actor with ActorLogging {

  val status = context.system.actorSelection("/user/status")

  val serial = new File(port)

  override def preStart() = {
    log.info(s"Sending status to serial port $port")
    status ! Status.Register(self)
  }

  def receive = {
    case message: StatusMessage =>
      val out = new PrintWriter(serial , "UTF-8")
      try {
        out.print(message.command)
      } catch {
        case t: Throwable =>
          log.error(s"Error writing to serial port $port", t)
      } finally {
        out.close()
      }
  }

}
