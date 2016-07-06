package net.rznc.smartpong.server

import akka.actor.{ActorRef, ActorSystem}
import akka.event.{Logging, LoggingAdapter}
import akka.http.scaladsl.Http
import akka.http.scaladsl.server.Directives._
import akka.pattern.ask
import akka.stream.{ActorMaterializer, Materializer}
import akka.util.Timeout
import com.typesafe.config.{Config, ConfigFactory}
import scala.concurrent.{ExecutionContextExecutor, Future}
import scala.concurrent.duration._

trait Service {

  implicit val system: ActorSystem
  implicit def executor: ExecutionContextExecutor
  implicit val materializer: Materializer

  def config: Config
  val logger: LoggingAdapter
  val actions: ActorRef

  implicit val timeout = Timeout(2.seconds)

  val routes = {
    logRequestResult("smartpong") {
      path("") {
        getFromResource("index.html")
      } ~
      path("actions") {
        get {
          complete {
            (actions ? ActionsActor.GetActions).asInstanceOf[Future[Seq[String]]] map { as =>
              as.mkString("\n")
            }
          }
        } ~
        (post & formFields('action)) { action =>
          actions ! ActionsActor.Action(action)
          complete("success")
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
  override val actions = system.actorOf(ActionsActor.props())

  Http().bindAndHandle(routes, config.getString("http.interface"), config.getInt("http.port"))

}
