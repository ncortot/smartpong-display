package net.rznc.smartpong.server

import akka.actor.{ActorRef, ActorSystem}
import akka.agent.Agent
import akka.event.{Logging, LoggingAdapter}
import akka.http.scaladsl.Http
import akka.http.scaladsl.model.ws._
import akka.http.scaladsl.server.Directives._
import akka.pattern.ask
import akka.stream.{ActorMaterializer, Materializer}
import akka.stream.actor.ActorPublisher
import akka.stream.scaladsl.{Flow, Sink, Source}
import akka.util.Timeout
import com.typesafe.config.{Config, ConfigFactory}
import scala.concurrent.duration._

import scala.concurrent.ExecutionContextExecutor

trait Service {

  implicit val system: ActorSystem
  implicit def executor: ExecutionContextExecutor
  implicit val materializer: Materializer

  implicit val timeout = Timeout(2.seconds)

  def config: Config
  val logger: LoggingAdapter

  val controlActor: ActorRef
  val displayActor: ActorRef
  val scoreAgent: Agent[Score]

  lazy val controlSink: Sink[Message, _] = {
    Sink.actorRef(controlActor, 'shutdown)
  }

  def displaySource(): Source[Message, _] = {
    val clientActor = system.actorOf(DisplayActor.props(scoreAgent))
    displayActor ! clientActor
    val publisher = ActorPublisher[Score](clientActor)
    Source.fromPublisher(publisher).map { score =>
      TextMessage(score.toString)
    }
  }

  val routes = {
    logRequestResult("smartpong") {
      path("") {
        getFromResource("index.html")
      } ~
      (path("actions") & post & formFields('action)) { action =>
        complete((controlActor ? action).map(_.toString))
      } ~
      pathPrefix("ws") {
        path("control") {
          handleWebSocketMessages(Flow.fromSinkAndSource(controlSink, displaySource()))
        } ~
        path("display") {
          handleWebSocketMessages(Flow.fromSinkAndSource(Sink.ignore, displaySource()))
        }
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

  override val scoreAgent = Agent(Score())
  override val displayActor = system.actorOf(RefreshActor.props())
  override val controlActor = system.actorOf(ControlActor.props(displayActor, scoreAgent))

  Http().bindAndHandle(routes, config.getString("http.interface"), config.getInt("http.port"))

}
