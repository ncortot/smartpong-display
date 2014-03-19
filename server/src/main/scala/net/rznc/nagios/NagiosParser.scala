package net.rznc.nagios

import java.io.ByteArrayInputStream
import org.jsoup.Jsoup
import org.jsoup.nodes.Element
import scala.collection.JavaConversions._

trait NagiosParser {

  case class NagiosService(host: String, name: String)

  case class NagiosStatus(
    critical: Set[NagiosService] = Set.empty[NagiosService],
    warning: Set[NagiosService] = Set.empty[NagiosService],
    okCount: Int = 0
  )

  def parse(data: Array[Byte]): NagiosStatus = {
    val document = Jsoup.parse(new ByteArrayInputStream(data), null, "")
    val (status, _) = document.select("table.status > tbody > tr")
      .foldLeft((NagiosStatus(), "unknown"))({ (result: (NagiosStatus, String), tr: Element) =>
        val cells = tr.children()
        if (cells.size() > 2 && cells.first().tagName() == "td") {
          val (status, prevHost) = result
          val ack = !tr.select("img[src$=/ack.gif]").isEmpty
          val downtime = !tr.select("img[src$=/downtime.gif]").isEmpty
          if (ack || downtime) {
            (status.copy(okCount = status.okCount + 1), prevHost)
          } else {
            val host = cells.get(0).select("a").map(_.text()).lastOption.getOrElse(prevHost)
            val name = cells.get(1).select("a").first().text()
            val service = NagiosService(host, name)
            if (cells.get(2).text() == "CRITICAL")
              (status.copy(critical = status.critical + service), host)
            else
              (status.copy(warning = status.warning + service), host)
          }
        } else {
          result
        }
    })
    val okCount = document.select("td.serviceTotalsOk").first().text().toInt
    status.copy(okCount = status.okCount + okCount)
  }

}
