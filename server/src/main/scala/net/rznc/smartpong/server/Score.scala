package net.rznc.smartpong.server

object Score {

  def apply(): Score = Score(0, 0, 0, 0, Undefined)

  sealed abstract class Service(side: Int)
  case object Undefined extends Service(0)
  case object Player1 extends Service(1)
  case object Player2 extends Service(2)

}

case class Score(
  p1: Int,
  p2: Int,
  s1: Int,
  s2: Int,
  service: Score.Service
)
