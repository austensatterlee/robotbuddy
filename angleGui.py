from __future__ import division
import Tkinter as tk
import numpy as np
import time,sys,re,fileinput
import transient

# PyQT4 imports
from PyQt4 import QtGui, QtCore, QtOpenGL
from PyQt4.QtOpenGL import QGLWidget
# PyOpenGL imports
import OpenGL.GL as gl
import OpenGL.GLUT as glut
import OpenGL.arrays.vbo as glvbo


class GLPlotWidget():
    # default window size
    def __init__(self,*args,**kwargs):
        """
        Keyword Arguments:
        dimensions - initial dimensions of GUI window
        data - signal data to plot
        view - left and right bounds specifying which samples should fill the screen first

        """
        self.init_time = time.time()
        self.width,self.height = kwargs.get('dimensions',(800,600))
        self.sampleres = self.width*4+1

        self.camerazoom = [0.95,0.95]
        self.zoom = [1.0,1.0]
        self.pan = [0.0,0.0]

        self.signals = []
        self.signalnames = []
        self.vbo = []
        self.g_signallength = 0.0
        self.g_signalheight = [0.0,0.0]
        self.left,self.right = 0.0,1.0
        if kwargs.get('data')!=None:
            self.addSignal(kwargs.get('data'))
        self.left,self.right = kwargs.get('view') or (0,self.g_signallength)
        self.isDirty = True

        self.pctpersample = 0

        inputFuncs = {
                (('KB',ord('q')),):lambda:sys.exit(0),
                }
        self.inputHandler = InputHandler(inputFuncs)

        self.axes = GLAxes(kwargs.get('nxticks'),10,self)

        self.configParams(**kwargs)

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

        self.userZoomWindow(0);
        self.userShiftWindow(0);


    def set_data(self,left=None,right=None):
        """
        Load 2D data as a Nx2 Numpy array.
        """
        self.left = left or self.left
        self.right = right or self.right
        if self.left%1!=0 or self.right%1!=0:
            self.left,self.right = self.left*self.g_signallength,self.right*(self.g_signallength)-1
            self.left = int(self.left)
            self.right = int(self.right)
        if self.right==0:
            self.right = self.g_signallength-1
        self.samplecount = self.right-self.left
        signal_height = (self.g_signalheight[1]-self.g_signalheight[0])
        self.pctpersample = np.array((2.0/self.samplecount,1.0/signal_height))

        for i in xrange(len(self.signals)):
            # Draw an extra sample or two offscreen so the line doesn't stop
            # short
            displeft = min(self.left,self.signals[i].size-1)
            dispright = min(self.right+1,self.signals[i].size-1)

            if (self.right-self.left)>self.sampleres:
                display_samples = np.linspace(displeft,dispright,self.sampleres*2,endpoint=True)
            else:
                display_samples = (displeft,dispright)

            display_samples = np.array(display_samples,dtype=np.uint)

            try:
                vbodata = np.array(
                        self.toScreen(
                            display_samples,i
                            ),dtype=np.float32)

            except ValueError,e:
                print e
                import pdb;pdb.set_trace()
            self.vbo[i] = glvbo.VBO(vbodata)

    def paintGL(self):
        """
        Paint the scene.
        """
        # clear the buffer
        gl.glClear(gl.GL_COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT)

        self.inputHandler.runInputFuncs()

        if self.isDirty and len(self.signals)>0:
            self.set_data(self.left,self.right)
        self.isDirty = False


        gl.glPushMatrix()
        gl.glScalef(self.camerazoom[0],self.camerazoom[1],1.0)
        self.axes.paint()

        """ Draw signals """
        for i in xrange(len(self.signals)):
            # bind the VBO
            self.vbo[i].bind()
            # tell OpenGL that the VBO contains an array of vertices
            gl.glEnableClientState(gl.GL_VERTEX_ARRAY)
            # these vertices contain 2 single precision coordinates
            gl.glVertexPointer(2, gl.GL_FLOAT, 0, self.vbo[i])
            # draw "count" points from the VBO
            color = self.getColor(i)

            gl.glPushMatrix()
            gl.glColor4f(color[0],color[1],color[2],1.0)
            gl.glLineWidth(1.0)
            gl.glDrawArrays(gl.GL_LINE_STRIP, 0, len(self.vbo[i]))
            gl.glPopMatrix()

            # gl.glPushMatrix()
            # ptsize = np.pi*((self.right-self.left)/self.g_signallength)**2
            # ptsize = min(max(ptsize,1),10)
            # gl.glPointSize(ptsize)
            # gl.glColor4f(color[0],color[1],color[2]/2,1.00)
            # gl.glDrawArrays(gl.GL_POINTS, 0, len(self.vbo[i]))
            # gl.glPopMatrix()

        self.drawLegend(0.8,0.9)
        gl.glPopMatrix()
        self.drawHUD()

        glut.glutSwapBuffers()

    def resizeGL(self, width, height):
        """
        Called upon window resizing: reinitialize the viewport.
        """
        # update the window size
        self.width, self.height = width, height
        self.sampleres = self.width*4+1
        # paint within the whole window
        gl.glViewport(0, 0, self.width,self.height)
        # set orthographic projection (2D only)
        gl.glMatrixMode(gl.GL_PROJECTION)
        gl.glLoadIdentity()
        # the window corner OpenGL coordinates are (-+1, -+1)
        gl.glOrtho(-1, 1, -1, 1, -1, 1)

    def drawHUD(self,):
        if len(self.signals)>0:
            adj_left,adj_right,adj_middle = self.getSignalBounds()
            pctseen = [
                    '%{:.4f}'.format(p) 
                    for p in 
                    np.array((adj_left,adj_right))/self.g_signallength
                    ]
            pctseen = ' '.join(pctseen)
            locstr = "[{} {}] [{}]".format(adj_left,adj_right,pctseen)

            mousex,mousey = self.getMousePos()
            mousesample = self.toSample(mousex)
            mousestr = "Sample: {0:}, ".format(mousesample)
            mousestr += "{}".format(mousey/self.pctpersample[1])

            self.drawText(-1.0,.9,'\n'.join([locstr,mousestr]))

        else:
            self.drawText(-.50,.25,"No signal loaded!",4.0)
        self.drawText(-1.0,.7,"Input: {}".format(hex(self.inputHandler.inputEvents)))
        self.drawText(0.0,-.9,"g_Wxg_H: {}x{}".format(self.g_signallength,self.g_signalheight))

    def getMousePos(self,):
        viewdim = np.array([self.width,self.height])
        cam_mouse = self.inputHandler.mouse[:,self.inputHandler.ind]

        mousepos = np.array([cam_mouse[0]-viewdim[0]/2,viewdim[1]/2-cam_mouse[1]])*2/viewdim
        mousepos = mousepos / self.camerazoom
        mousepos[0] = max(min(mousepos[0],1.0),-1.0)
        mousepos[1] = max(min(mousepos[1],1.0),-1.0)
        return mousepos

    def drawBox(self,p1,p2):
        gl.glPushMatrix()
        gl.glBegin(gl.GL_QUAD_STRIP)
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

class GLAxes(object):
    def __init__(self,nxticks,nyticks,parentwidget,*args,**kwargs):
        self.parent = parentwidget
        self.xtickvbo = None
        self.ytickvbo = None

        self.nxticks = int(nxticks) if nxticks else int(self.parent.width/10.0)
        self.nyticks = int(nyticks) if nyticks else int(self.parent.height/10.0)
        self.ticklength = 5

        self.c_tick = np.array([0.0,0.0,0.0,1.00])
        self.c_axis = np.array([0.0,0.0,0.0,0.90])
        self.c_grid = np.array([0.0,0.02,0.09,0.10])

    def paint(self):
        gl.glPushMatrix()
        """ Draw the x-axis line """
        gl.glLineWidth(1.00)
        gl.glColor4f(*self.c_axis)
        gl.glBegin(gl.GL_LINE_STRIP)
        gl.glVertex2d(-1,0)
        gl.glVertex2d(1,0)
        gl.glEnd()
        gl.glFlush()

        """ Draw the x-axis ticks """
        numonscreen = self.parent.right-self.parent.left
        gl.glColor4f(*self.c_tick)
        gl.glBegin(gl.GL_LINES)
        i = self.parent.left
        while i < self.parent.right:
            xtick = self.parent.toScreen(np.array([i]))[0]
            gl.glVertex2d(xtick,self.ticklength/self.parent.height)
            gl.glVertex2d(xtick,-self.ticklength/self.parent.height)
            i+=int(np.ceil(numonscreen/self.nxticks))
        gl.glEnd()
        gl.glFlush()


        """ Draw the x-axis grid lines (i.e. vertical) """
        gl.glColor4f(*self.c_grid)
        gl.glBegin(gl.GL_LINES)
        i = self.parent.left
        xticklist=[]
        while i < self.parent.right:
            xtick = self.parent.toScreen(np.array([i]))[0]
            xticklist.append((i,xtick))
            gl.glVertex2d(xtick,1.0)
            gl.glVertex2d(xtick,-1.0)
            i+=int(np.ceil(numonscreen/self.nxticks))
        gl.glEnd()
        gl.glFlush()
        """ Draw the x-axis tick labels """
        #if len(xticklist)>1:
            #i=0
            #tickdiffdigits = len(str(int((self.parent.right-self.parent.left))))
            #for xticklabel,xtickpos in xticklist:
                #if(i%tickdiffdigits==0):
                    #self.parent.drawText(xtickpos, -0.1, "{:d}".format(xticklabel),scale=1.0)
                #i+=1

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

def main(**kwargs):
    bits = kwargs.get('bits')
    x = GLPlotWidget(nxticks=kwargs.get('nxticks'),view=kwargs.get('view'))
    print "Loading..."
    if kwargs.get('commandfile'):
        commands = ""
        for line in fileinput.input(kwargs.get('commandfile')):
            commands+=line.strip()+"|"
        commands = commands.rstrip("|")
        kwargs["command"]=commands

    alter = []
    if kwargs.get('alter'):
        commands = ""
        alter = kwargs.get('alter').split(";")

    if kwargs.get('wavfile'):
        data,sr = transient.makewindows(kwargs.get('wavfile'),bits=bits,channel=0)
        data = np.hstack(data);
        # data = (data-data.min())/data.ptp()

        # energy = transient.energy(data,sr/1000.0*50)

        # Strip path and extension from filename
        signalname = re.match(r"(?:.*[\\/])?([^.]+)",kwargs.get('wavfile')).groups()[0] 

        if alter:
            alterfunc=eval(alter[0])
            data = alterfunc(data)
        x.addSignal(data,signalname)
        if kwargs.get('split')!=None:
            cutpts = transient.getOnsetsFromEnergy(energy,kwargs.get('split'))
            cutsignal = np.zeros(len(data))
            cutsignal[cutpts]=data.max()
            print "Cuts: {}".format(cutpts);
            x.addSignal(cutsignal,'cutsignal')
    elif kwargs.get('command'):
        commands = kwargs.get('command').split('|')
        if kwargs.get('verbose')>0:
            print commands
        for cmd in commands:
            data = eval(cmd)
            x.addSignal(data)
    else:
        print "No data was provided! I need either a filename or some code to execute. See -h for help"
        sys.exit(1)

    print "Done!"

    x.initialize()


if __name__ == '__main__':
    import argparse
    arguments = sys.argv[1:]
    mainParser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    mainParser.add_argument('-f','--wavfile',type=str)
    mainParser.add_argument('-a','--alter',type=str,help="apply the given lambda function to the waveform retrieved from -f")
    mainParser.add_argument('-c','--command',type=str,help="plot the data resulting from the executing the provided python code")
    mainParser.add_argument('-cc','--commandfile',type=str)
    mainParser.add_argument('-b','--bits',type=int,help="level of quantization to perform",default=0)
    mainParser.add_argument('-nx','--nxticks',type=int,help="number of ticks on the axis",default=None)
    mainParser.add_argument('-w','--view',type=str,help="left and right bounds specifying which samples should fill the view first",default=None)
    mainParser.add_argument('-s','--split',type=int,help="if present, will split the wav file based on detected onsets from the given tolerance",default=None)
    mainParser.add_argument('-v','--verbose',type=int,help="specify verbosity level",default=0)

    parsedargs = vars(mainParser.parse_args())
    parsedargs['view'] = map(float,parsedargs['view'].split(',')) if parsedargs['view'] else None
    main(**parsedargs)
