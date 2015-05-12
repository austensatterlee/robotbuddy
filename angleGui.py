from __future__ import division
import Tkinter as tk
import numpy as np
import time,sys,re,fileinput
import struct

# PyQT4 imports
from PyQt4 import QtGui, QtCore, QtOpenGL
from PyQt4.QtOpenGL import QGLWidget
# PyOpenGL imports
import OpenGL.GL as gl
import OpenGL.GLUT as glut
import OpenGL.arrays.vbo as glvbo


class GLPlotWidget():
    # default window size
    def __init__(self,stream,*args,**kwargs):
        """
        Keyword Arguments:
        dimensions - initial dimensions of GUI window
        data - signal data to plot
        view - left and right bounds specifying which samples should fill the screen first

        """
        self.init_time = time.time()
        self.width,self.height = kwargs.get('dimensions',(800,600))
        self.stream = stream

        self.zoom = [1.0,1.0]
        self.pan = [0.0,0.0]
        self.angle = 45.
        self.PIDoutput = 0.

        self.lastSampleTime = time.time()
        self.samplePeriod = 1

        self.isDirty = True

        inputFuncs = {
                (('KB',ord('q')),):self.close,
                }
        self.inputHandler = InputHandler(inputFuncs)
        self.configParams(**kwargs)

    def close(self):
        self.stream.close()
        sys.exit(0)

    def configParams(self,*args,**kwargs):
        self.panspeed = kwargs.get('panspeed',1.0e-1)
        self.zoomspeed = kwargs.get('zoomspeed',5.0e-1)
        self.c_palette = [
                            (0.41221438595965454, 0.0, 0.0),
                            (0.79312361597374381, 0.0, 0.0),
                            (1.0, 0.16372618247436524, 0.0),
                            (1.0, 0.544607916482724, 0.0),
                            (1.0, 0.91519554957193794, 0.0),
                            (1.0, 1.0, 0.44411709117591475)
                            ]
        self.c_bg = np.array([ 0.93,  0.92,  0.93,  1.0])
        self.c_legend = np.array([0.08,0.08,0.18,0.14])

    def getColor(self,i):
        return self.c_palette[i%len(self.c_palette)]

    def initialize(self):
        glut.glutInit()
        glut.glutInitDisplayMode(glut.GLUT_RGBA | glut.GLUT_DOUBLE | glut.GLUT_DEPTH)
        glut.glutInitWindowSize(self.width,self.height)
        glut.glutInitWindowPosition(300,300)
        glut.glutCreateWindow("super black hole sunshine party")
        glut.glutDisplayFunc(self.paintGL)
        glut.glutIdleFunc(self.paintGL)
        glut.glutReshapeFunc(self.resizeGL)
        self.initGL()

        self.inputHandler.attachGLUT()

        glut.glutMainLoop()

    def initGL(self):
        """Initialize OpenGL, VBOs, upload data on the GPU, etc.
        """
        # background color
        gl.glEnable( gl.GL_POINT_SMOOTH );
        gl.glEnable( gl.GL_LINE_SMOOTH );
        gl.glEnable(gl.GL_BLEND);
        gl.glBlendFunc(gl.GL_SRC_ALPHA, gl.GL_ONE_MINUS_SRC_ALPHA);
        gl.glClearColor(*self.c_bg)

    def updateAngle(self):
        # Read 4 bytes
        nextAngle = self.stream.read(8)
        if len(nextAngle)!=8:
            return
        serialData = struct.unpack("ff",nextAngle)
        nextAngle = serialData[0]
        pidOutput = serialData[1]
        self.angle = np.float(nextAngle*180/np.pi)
        self.PIDoutput = np.float(pidOutput)
        self.samplePeriod = time.time()-self.lastSampleTime
        self.lastSampleTime = time.time()

    def paintGL(self):
        self.updateAngle()
        """
        Paint the scene.
        """
        # clear the buffer
        gl.glClear(gl.GL_COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT)

        self.inputHandler.runInputFuncs()

        self.isDirty = False

        gl.glPushMatrix()
        gl.glScalef(1.0,1.0,1.0)

        """ Draw shape """
        gl.glRotatef(90-self.angle,0,0,1)
        self.drawBox([0,0],[0.5,0.01])
        gl.glPopMatrix()
        self.drawHUD()

        glut.glutSwapBuffers()

    def resizeGL(self, width, height):
        """
        Called upon window resizing: reinitialize the viewport.
        """
        # update the window size
        self.width, self.height = width, height
        # paint within the whole window
        gl.glViewport(0, 0, self.width,self.height)
        # set orthographic projection (2D only)
        gl.glMatrixMode(gl.GL_PROJECTION)
        gl.glLoadIdentity()
        # the window corner OpenGL coordinates are (-+1, -+1)
        gl.glOrtho(-1, 1, -1, 1, -1, 1)

    def drawHUD(self,):
        self.drawText(-1.0,.95,"Angle: {:<.8f}".format(self.angle))
        self.drawText(-1.0,.90,"PID Output: {:<.8f}".format(self.PIDoutput))
        self.drawText(-1.0,.85,"Sample Period: {:<.8f}".format(self.samplePeriod))
        self.drawText(-1.0,.8,"Input: {}".format(hex(self.inputHandler.inputEvents)))

    def getMousePos(self,):
        viewdim = np.array([self.width,self.height])
        cam_mouse = self.inputHandler.mouse[:,self.inputHandler.ind]

        mousepos = np.array([cam_mouse[0]-viewdim[0]/2,viewdim[1]/2-cam_mouse[1]])*2/viewdim
        mousepos = mousepos / self.camerazoom
        mousepos[0] = max(min(mousepos[0],1.0),-1.0)
        mousepos[1] = max(min(mousepos[1],1.0),-1.0)
        return mousepos

    def drawBox(self,p1,p2,c=[0,0,0,0.5]):
        gl.glPushMatrix()
        gl.glBegin(gl.GL_QUAD_STRIP)
        gl.glColor4f(c[0],c[1],c[2],c[3])
        gl.glVertex2d(p1[0],p1[1])
        gl.glVertex2d(p2[0],p1[1])
        gl.glVertex2d(p1[0],p2[1])
        gl.glVertex2d(p2[0],p2[1])
        gl.glEnd()
        gl.glFlush()
        gl.glPopMatrix()

    def drawText(self,x,y,text,scale=1.0,color=(0,0,0,1)):
        default_scale = 0.15
        gl.glPushMatrix()
        gl.glTranslate(x,y,0)
        gl.glScalef(default_scale/self.width*scale,default_scale/self.height*scale,1.0)
        gl.glLineWidth(1.0)
        gl.glColor4f(*color)
        glut.glutStrokeString(glut.GLUT_STROKE_MONO_ROMAN,text)
        gl.glPopMatrix()

class InputHandler(object):
    INPUT_SHIFTS = {
            'MOUSE': 0,
            'WHEEL':3,
            'SPECIAL':4,
            'KB':9,
            }

    IMAP = {
            'MOUSE': {
                'LEFTCLICK': 0,
                'MIDDLECLICK': 1,
                'RIGHTCLICK': 2,
                },
            'WHEEL': {
                'DOWN': 0,
                'UP': 1,
                },
            'SPECIAL': {
                'SHIFT': 12,
                'CTRL': 14,
                'ALT':16,
                },
            'KB':{

                }
            }

    @staticmethod
    def convert_input(eventtype,eventnum):
        if eventtype=='KB':
            inmap = eventnum
        else:
            inmap = InputHandler.IMAP[eventtype][eventnum]
        return 1<<(inmap+InputHandler.INPUT_SHIFTS[eventtype])

    @staticmethod
    def convert_inputmap(inputmap):
        converted = {}
        for inputkey in inputmap:
            converted_key = 0x000000
            for k in inputkey:
                converted_key |= InputHandler.convert_input(*k)
            converted[converted_key] = inputmap[inputkey]
        return converted

    def __init__(self,inputfunc_map):
        self.mouse = np.zeros((2,60))
        self.ind = 0
        self.inputEvents = 0x000000000L
        self.inputFuncs = InputHandler.convert_inputmap(inputfunc_map)

    def attachGLUT(self):
        glut.glutMouseFunc(self.mouseInputHandler)
        glut.glutMouseWheelFunc(self.mouseWheelHandler)

        glut.glutMotionFunc(self.mouseMoveHandler)
        glut.glutPassiveMotionFunc(self.mouseMoveHandler)

        glut.glutKeyboardFunc(self.keyboardHandler)
        glut.glutKeyboardUpFunc(lambda *x:self.keyboardHandler(*x,isEnd=True))

        glut.glutSpecialFunc(self.specialKeyHandler)
        glut.glutSpecialUpFunc(lambda *x:self.specialKeyHandler(*x,isEnd=True))

    def runInputFuncs(self):
        if self.inputEvents in self.inputFuncs:
            self.inputFuncs[self.inputEvents]()

    def recordInput(self,eventType,eventNum,isEnd=None):
        event = 1<<(eventNum+InputHandler.INPUT_SHIFTS[eventType])
        if isEnd==None:
            # isEnd = (self.inputEvents&event)==event
            self.inputEvents |= event
            self.runInputFuncs()
            # self.inputEvents &= event
            self.inputEvents ^= event
        elif isEnd:
            self.inputEvents &= event
            self.inputEvents ^= event
        else:
            self.inputEvents |= event

    def mouseInputHandler(self,*event):
        eventNum,isEnd,mouseX,mouseY = event
        self.recordInput('MOUSE',eventNum,isEnd)

    def mouseMoveHandler(self,*event):
        mousex,mousey = event
        self.ind = (self.ind+1)%len(self.mouse)
        self.mouse[0,self.ind] = mousex
        self.mouse[1,self.ind] = mousey

    def mouseWheelHandler(self,*event):
        mouseWheel,direction,mouseX,mouseY = event
        direction = 0 if direction==-1 else 1
        self.recordInput('WHEEL',direction,)

    def keyboardHandler(self,*event,**kwargs):
        key,mousex,mousey = event
        self.recordInput('KB',ord(key),isEnd=kwargs.get('isEnd',False))

    def specialKeyHandler(self,*event,**kwargs):
        keyid,mousex,mousey = event
        keyid-=100
        if keyid not in InputHandler.IMAP['SPECIAL'].values():
            self.inputEvents = 0x0L
            return
        self.recordInput('SPECIAL',keyid,isEnd=kwargs.get('isEnd',False))

def initializeGui(serial):
    x = GLPlotWidget(serial)
    print "Loading..."
    x.initialize()


