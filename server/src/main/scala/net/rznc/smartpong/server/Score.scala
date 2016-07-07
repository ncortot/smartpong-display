package net.rznc.smartpong.server

object Score {

  def apply(): Score = Score(0, 0, 0, 0, 0, 0)

}

case class Score(
  p1: Int,
  p2: Int,
  s1: Int,
  s2: Int,
  initialService: Int,
  currentService: Int
)
