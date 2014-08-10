package net.rznc.nagios

import akka.util.ByteString

object Commands {

  sealed abstract class Color
  case object GREEN extends Color
  case object ORANGE extends Color
  case object RED extends Color

  sealed abstract class NotificationType
  case object OK extends NotificationType
  case object WARNING extends NotificationType
  case object CRITICAL extends NotificationType

  trait StatusMessage {

    def command: String

    def toByteString: ByteString = ByteString(command.getBytes)

  }

  case class Message(message: String, color: Color = ORANGE) extends StatusMessage {

    def command: String = s"COLOR $color\n$message\n"

  }

  case class Notification(notification: NotificationType) extends StatusMessage {

    def command: String = s"NOTIFY $notification\n"

  }

  case class Update(critical: Int = 0, warning: Int = 0, ok: Int = 0) extends StatusMessage {

    def command: String = s"UPDATE $critical $warning $ok\n"

  }

}
