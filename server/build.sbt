import AssemblyKeys._

name := "spark-nagios-monitor"

version := "2.0"

resolvers += "Typesafe Repository" at "http://repo.typesafe.com/typesafe/releases/"

libraryDependencies ++= Seq(
  "com.typesafe.akka" %% "akka-actor" % "2.3.4",
  "com.typesafe.akka" %% "akka-testkit" % "2.3.4",
  "org.jsoup" % "jsoup" % "1.7.3",
  "org.scalatest" % "scalatest_2.10" % "2.0.M5b" % "test",
  "io.spray" % "spray-client" % "1.3.1"
)

assemblySettings
