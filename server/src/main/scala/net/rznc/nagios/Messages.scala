package net.rznc.nagios

import akka.util.ByteString
import java.nio.ByteBuffer

object Messages {

  val STATUS_UPDATE = 0x00000001
  val STATUS_ERROR  = 0x00000002

  case class Counts(critical: Int = 0, warning: Int = 0, ok: Int = 0)

  case class Serials(critical: Int = 0, warning: Int = 0, ok: Int = 0)

  abstract class StatusMessage(val messageType: Int) {
    def toIntArray: Array[Int]

    def toByteString: ByteString = {
      val values = toIntArray
      val buffer = ByteBuffer.allocate(values.size * 4)
      values.foreach(i => buffer.putInt(i))
      buffer.flip
      ByteString(buffer)
    }
  }

  case class StatusUpdate(
    counts: Counts = Counts(),
    serials: Serials = Serials()
  ) extends StatusMessage(STATUS_UPDATE) {
    def toIntArray =
      Array[Int](
        messageType,
        counts.critical,
        counts.warning,
        counts.ok,
        serials.critical,
        serials.warning,
        serials.ok
      )
  }

  case class StatusError() extends StatusMessage(STATUS_ERROR) {
    def toIntArray = Array[Int](messageType)
  }

}
