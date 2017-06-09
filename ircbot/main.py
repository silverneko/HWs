#!/usr/bin/env python2

import socket
import random
import requests
import subprocess

def calc(expr):
    pp = subprocess.Popen("./calc",
                          stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    res = pp.communicate(expr)[0].rstrip('\r\n ')
    return res[:400]

def main():
    Server = ('localhost', 'irc.freenode.net')[1]
    Port = 6667
    Nick = 'yobot'
    Password = 'password'
    SelfIntroduction = "Hi, I'm %s!" % Nick
    YoutubeURL = 'https://www.youtube.com'
    YoutubeQuery = 'https://www.youtube.com/results?search_query='
    for config in open('config', 'r').readlines():
        config = config.rstrip('\r\n').split('=')
        if config[0] == 'CHAN':
            Chan = eval(config[1])
        elif config[0] == 'CHAN_KEY':
            ChanKey = eval(config[1])

    Socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    Socket.connect((Server, Port))
    Socket.send('PASS %s\r\n' % (Password))
    Socket.send('NICK %s\r\n' % (Nick))
    Socket.send('USER %s 0 0 :%s\r\n' % (Nick, Nick))
    #Socket.send('PRIVMSG NickServ :IDENTIFY yobot password\r\n')
    Socket.send('JOIN %s %s\r\n' % (Chan, ChanKey))
    Socket.send('PRIVMSG %s :%s\r\n' % (Chan, SelfIntroduction))

    GuessNumberState = [False, '', 0, 0]
    while True:
        msgs = Socket.recv(2048).split('\r\n')
        for msg in msgs:
            print msg
            if msg.startswith('PING'):
                Socket.send('PONG %s\r\n' % (msg[5:]))
            elif msg.find('PRIVMSG') != -1:
                msg = msg.split(' ')
                Sender = msg[0][1:].split('!')[0].split('@')[0]
                SendTo = msg[2]
                Message = ' '.join(msg[3:]).lstrip(': ')
                if Message.startswith('@repeat '):
                    Message = Message[len('@repeat '):]
                    Socket.send('PRIVMSG %s :%s (%s)\r\n'
                                % (Chan, Message, Sender))
                elif Message.startswith('@cal '):
                    Message = calc(Message[len('@cal '):])
                    Socket.send('PRIVMSG %s :%s (%s)\r\n'
                                % (Chan, Message, Sender))
                elif Message.startswith('@play '):
                    PlayWith = Message[len('@play '):].strip(' ')
                    if PlayWith != Nick:
                        continue
                    if GuessNumberState[0]:
                        continue
                    GuessNumberState = [True, Sender, 5, random.randint(1, 100)]
                    Socket.send('PRIVMSG %s :%s (%s)\r\n'
                    % (Chan, 'Start! (1-100 with 5 times)', Sender))
                elif Message.startswith('@guess '):
                    if not GuessNumberState[0]:
                        continue
                    Player = GuessNumberState[1]
                    if Player != Sender:
                        continue
                    Message = Message[len('@guess '):]
                    try:
                        Guess = int(Message)
                    except:
                        Socket.send('PRIVMSG %s :%s (%s)\r\n'
                                    % (Chan, 'Error: Bad input format', Sender))
                        continue
                    GuessNumberState[2] -= 1
                    Chance = GuessNumberState[2]
                    if Chance == 0:
                        GuessNumberState[0] = False
                    Answer = GuessNumberState[3]
                    if Guess < Answer:
                        if Chance == 0:
                            Socket.send('PRIVMSG %s :%s (%s)\r\n'
                                        % (Chan, 'Game over', Sender))
                        else:
                            Socket.send('PRIVMSG %s :%s (%s) (%s)\r\n'
                                        % (Chan, 'Higher', Chance, Sender))
                    elif Guess > Answer:
                        if Chance == 0:
                            Socket.send('PRIVMSG %s :%s (%s)\r\n'
                                        % (Chan, 'Game over', Sender))
                        else:
                            Socket.send('PRIVMSG %s :%s (%s) (%s)\r\n'
                                        % (Chan, 'Lower', Chance, Sender))
                    else:
                        GuessNumberState[0] = False
                        Socket.send('PRIVMSG %s :%s (%s)\r\n'
                                    % (Chan, 'Correct', Sender))
                elif Message.startswith('@youtube '):
                    Message = Message[len('@youtube '):]
                    try:
                        Respond = requests.get(YoutubeQuery + Message).text
                    except Exception, e:
                        Socket.send('PRIVMSG %s :%s (%s)\r\n'
                                    % (Chan, 'Error: %s' % e.reason, Sender))
                    else:
                        Pos = Respond.find('yt-lockup-title')
                        if Pos == -1:
                            Socket.send('PRIVMSG %s :%s (%s)\r\n'
                                        % (Chan, 'Not found', Sender))
                        else:
                            Pos = Respond.find('href', Pos)
                            Song = Respond[Pos:].split('"')[1]
                            Socket.send('PRIVMSG %s :%s (%s)\r\n'
                                        % (Chan, YoutubeURL + Song, Sender))

                elif Message.startswith('@help'):
                    Socket.send('PRIVMSG %s :%s (%s)\r\n'
                                % (Chan, '@repeat <String>', Sender))
                    Socket.send('PRIVMSG %s :%s (%s)\r\n'
                                % (Chan, '@cal <Expression>', Sender))
                    Socket.send('PRIVMSG %s :%s (%s)\r\n'
                                % (Chan, '@play <Robot Nick>', Sender))
                    Socket.send('PRIVMSG %s :%s (%s)\r\n'
                                % (Chan, '@guess <Integer>', Sender))
                    Socket.send('PRIVMSG %s :%s (%s)\r\n'
                                % (Chan, '@youtube <Song>', Sender))
                    Socket.send('PRIVMSG %s :%s (%s)\r\n'
                                % (Chan, '@help', Sender))



if __name__ == '__main__':
    main()
