package net.rznc.smartpong.server

import akka.actor.ActorRef
import akka.actor.ActorSystem
import akka.event.Logging
import akka.event.LoggingAdapter
import akka.http.scaladsl.Http
import akka.http.scaladsl.model.ws.Message
import akka.http.scaladsl.model.ws.TextMessage
import akka.http.scaladsl.server.Directives._
import akka.pattern.ask
import akka.stream.ActorMaterializer
import akka.stream.Materializer
import akka.stream.OverflowStrategy
import akka.stream.scaladsl.Flow
import akka.stream.scaladsl.Sink
import akka.stream.scaladsl.Source
import akka.util.Timeout
import com.typesafe.config.Config
import com.typesafe.config.ConfigFactory
import spray.json._
import scala.concurrent.duration._
import scala.concurrent.ExecutionContextExecutor

trait JsonProtocol extends DefaultJsonProtocol {

  implicit object serviceFormat extends RootJsonFormat[Score.Service] {
    def write(service: Score.Service) =
      JsNumber(service.side)
    def read(value: JsValue) = value match {
      case _ => deserializationError("Not supported")
    }
  }

  implicit val scoreFormat = jsonFormat5(Score.apply)

}

trait Service extends JsonProtocol {

  implicit val system: ActorSystem
  implicit def executor: ExecutionContextExecutor
  implicit val materializer: Materializer

  implicit val timeout = Timeout(2.seconds)

  def config: Config
  val logger: LoggingAdapter

  val controlActor: ActorRef
  val displayActor: ActorRef

  def controlFlow(): Flow[Message, Message, _] = {
    val in = Flow[Message]
      .mapConcat {
        case TextMessage.Strict(data) => ControlActor.Command(data) :: Nil
        case _ => Nil
      }
      .to(Sink.actorRef[ControlActor.Action](controlActor, ControlActor.Disconnected))

    val out = Source.actorRef[Score](bufferSize = 1, OverflowStrategy.dropHead)
      .mapMaterializedValue { ref =>
        displayActor ! ref
      }
      .map { score =>
        TextMessage(score.toJson.toString)
      }

    Flow.fromSinkAndSource(in, out)
  }

  val routes = {
    logRequestResult("smartpong") {
      path("") {
        getFromResource("index.html")
      } ~
      (path("actions") & post & formFields('action)) { action =>
        complete((controlActor ? ControlActor.Command(action)).map(_.toString))
      } ~
      path("ws") {
        handleWebSocketMessages(controlFlow())
      }
    }
  }

}

object SmartPongService extends App with Service {

  override implicit val system = ActorSystem()
  override implicit val executor = system.dispatcher
  override implicit val materializer = ActorMaterializer()

  override val config = ConfigFactory.load()
  override val logger = Logging(system, getClass)

  override val displayActor = system.actorOf(DisplayActor.props())
  override val controlActor = system.actorOf(ControlActor.props(displayActor))

  Http().bindAndHandle(routes, config.getString("http.interface"), config.getInt("http.port"))

}
