package net.rznc.nagios

import java.io.ByteArrayInputStream
import org.jsoup.Jsoup
import org.jsoup.nodes.Element
import scala.collection.JavaConversions._

object NagiosParser {

  case class NagiosService(host: String, name: String)

  case class NagiosStatus(
    critical: Set[NagiosService] = Set.empty[NagiosService],
    warning: Set[NagiosService] = Set.empty[NagiosService],
    ok: Int = 0
  )

  def parse(data: Array[Byte]): NagiosStatus = {
    val document = Jsoup.parse(new ByteArrayInputStream(data), null, "")
    val (status, _) = document.select("table.status > tbody > tr")
      .foldLeft((NagiosStatus(), "unknown"))({ (result: (NagiosStatus, String), tr: Element) =>
        val cells = tr.children
        if (cells.size > 2 && cells.first.tagName == "td") {
          result match {
            case (NagiosStatus(critical, warning, ok), prevHost) =>
              val host = cells.get(0).select("a").headOption.map(_.text).getOrElse(prevHost)
              val ack = tr.select("img[src$=/ack.gif]").nonEmpty
              val downtime = tr.select("img[src$=/downtime.gif]").nonEmpty
              if (ack || downtime) {
                (NagiosStatus(critical, warning, ok + 1), host)
              } else {
                val name = cells.get(1).select("a").first.text
                val service = NagiosService(host, name)
                if (cells.get(2).text() == "CRITICAL")
                  (NagiosStatus(critical + service, warning, ok), host)
                else
                  (NagiosStatus(critical, warning + service, ok), host)
              }
          }
        } else {
          result
        }
    })
    val other = document.select("td.serviceTotalsOk").first.text.toInt
    status.copy(ok = status.ok + other)
  }

}
