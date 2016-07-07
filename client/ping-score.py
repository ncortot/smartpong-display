#!/usr/bin/env python2.7
import urllib2
import time


COMMANDS_URL = 'http://smartpong.rznc.net/actions'


class Scorer(object):
    sets = [0, 0]
    score = [0, 0]
    service = 0
    first_service = 0

    stop = False

    def run(self):
        stop = False
        while not stop:
            data = urllib2.urlopen(COMMANDS_URL).read(10000)
            for command in data.split('\n'):
                self.parse_command(command)
            time.sleep(0.5)

    def parse_command(self, command):
        words = command.split()
        if len(words) == 0:
            return
        cmd = words[0]
        if cmd == 'stop':
            self.stop = True
        elif cmd == 'reset':
            self.reset()
            if len(words) > 1:
                self.service = int(words[1])
                self.first_service = self.service
        elif cmd == 'srv1':
            self.service = 1
            if self.sets == [0, 0] and self.score == [0, 0]:
                self.first_service = self.service
        elif cmd == 'srv2':
            self.service = 2
            if self.sets == [0, 0] and self.score == [0, 0]:
                self.first_service = self.service
        elif cmd == 'score':
            if len(words) < 3:
                return
            self.score = [int(words[1]), int(words[2])]
        elif cmd == 'sets':
            if len(words) < 3:
                return
            self.sets = [int(words[1]), int(words[2])]
        elif cmd == 's1+':
            self.sets[0] = self.sets[0] + 1
        elif cmd == 's1-':
            self.sets[0] = max(0, self.sets[0] - 1)
        elif cmd == 's2+':
            self.sets[1] = self.sets[1] + 1
        elif cmd == 's2-':
            self.sets[1] = max(0, self.sets[1] - 1)
        elif cmd == 'p1+':
            self.add_point(0)
        elif cmd == 'p1-':
            self.sub_point(0)
        elif cmd == 'p2+':
            self.add_point(1)
        elif cmd == 'p2-':
            self.sub_point(1)
        elif cmd == 'swap':
            self.sets.reverse()
            self.score = [0, 0]
            self.service = self.first_service
        self.draw()

    def add_point(self, player):
        other = 1 if player == 0 else 0
        self.score[player] += 1
        if self.score[player] >= 11 and self.score[player] - self.score[other] > 1:
            self.sets[player] += 1
            return
        if self.score[0] >= 10 and self.score[1] >= 10:
            self.service = 2 if self.service == 1 else 1
        elif (self.score[player] + self.score[other]) % 2 == 0:
            self.service = 2 if self.service == 1 else 1

    def sub_point(self, player):
        other = 1 if player == 0 else 0
        if self.score[player] <= 0:
            return
        self.score[player] -= 1
        # TODO: fix code below
        if self.score[player] >= 11 and self.score[player] - self.score[other] > 1:
            self.sets[player] += 1
            return
        if self.score[0] >= 10 and self.score[1] >= 10:
            self.service = 2 if self.service == 1 else 1
        elif (self.score[player] + self.score[other]) % 2 == 0:
            self.service = 2 if self.service == 1 else 1

    def reset(self):
        self.sets = [0, 0]
        self.score = [0, 0]
        self.service = 0
        self.first_service = 0

    def draw(self):
        out = '%d %2d %2d %d %d' % (self.sets[0], self.score[0],
                                   self.score[1], self.sets[1],
                                   self.service)
        print "OUT: ", out
        f = open('/dev/ttyAMA0', 'w')
        f.write(out + '\n')


scorer = Scorer()
scorer.run()
