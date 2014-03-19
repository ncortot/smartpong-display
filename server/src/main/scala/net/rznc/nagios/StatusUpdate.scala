package net.rznc.nagios

import akka.util.ByteString
import java.nio.ByteBuffer

case class StatusUpdate(
  criticalCount: Int = 0,
  criticalSerial: Int = 0,
  warningCount: Int = 0,
  warningSerial: Int = 0,
  okCount: Int = 0,
  okSerial: Int = 0
) {
  def toByteString: ByteString = {
    val buffer = ByteBuffer.allocate(24)
      .putInt(criticalCount)
      .putInt(criticalSerial)
      .putInt(warningCount)
      .putInt(warningSerial)
      .putInt(okCount)
      .putInt(okSerial)
    buffer.flip
    ByteString(buffer)
  }
}
