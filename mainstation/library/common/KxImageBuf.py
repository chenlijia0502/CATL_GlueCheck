#coding=utf-8
'''
Created on 2015年10月8日

@author: ljj
'''
#from src.codes.kxpackage.image.display.KxImage import KxMvImage
import struct
from PIL import Image
import os
import numpy as np

class KxImageBuf(object):
    '''
    classdocs
    '''


    def __init__(self):
        '''
        Constructor
        '''
        self.clear()
    
    def clear(self): 
        self.nWidth = 0
        self.nHeight = 0
        self.nPitch = 0
        self.buf = ''
        self.bAuto = False
        self.nChannel = 0
        
        self.notEqualInfStr = ''
        
            
    def Release(self): 
        self.clear() 
        
    def SetImageBuf(self, buffer, width, height, pitch, nChan, bCopy):
        self.Release()
        self.nWidth = width
        self.nHeight = height
        self.bAuto = bCopy
        self.nChannel = nChan
        
        if self.bAuto:
            self.nPitch = self.nWidth * self.nChannel
            for index in range(height):
                self.buf += buffer[index * pitch : index * pitch + width * nChan]
        else:
            self.nPitch = pitch
            self.buf = buffer
            
    def GetImageBuf(self):
        mvImage = KxMvImage()
        mvImage.LoadImage(self.buf, self.nWidth, self.nHeight, self.nPitch, self.ConverType(self.nChannel))
        
        return mvImage  

    def pack(self): 
        nToken = 0
        if len(self.buf) > 0:
            nToken = 1
        fmt =  '%ds' %(self.nHeight * self.nPitch)
        return struct.pack('=5i' + fmt, self.nWidth, self.nHeight, self.nPitch, self.nChannel, nToken, self.buf) 
        
    def unpack(self, str): 
        fmt = '5i'
        presize = struct.calcsize(fmt)
        
        self.nWidth, self.nHeight, self.nPitch, self.nChannel, nToken = struct.unpack(fmt, str[:presize])
        fmt1 = ''
     

        if nToken == 1:
            fmt1 = '=%ds' %(self.nHeight * self.nPitch)

            self.buf = str[presize : presize + struct.calcsize(fmt1)]
        return str[presize + struct.calcsize(fmt1):]
    
    
    
    def ConverType(self, nChannel):
        pass
        # if nChannel == 1:
        #     return KxMvImage.Type.MV_IMAGE_TYPE_G8
        # elif nChannel == 3:
        #     return KxMvImage.Type.MV_IMAGE_TYPE_RGB24
        # elif nChannel == 4:
        #     return KxMvImage.Type.MV_IMAGE_TYPE_RGB32
        # else:
        #     return KxMvImage.Type.MV_IMAGE_TYPE_INVALID
                
    def GetChannel(self, mvImageType):
        pass
        # if mvImageType == KxMvImage.Type.MV_IMAGE_TYPE_G8:
        #     return 1
        # elif mvImageType == KxMvImage.Type.MV_IMAGE_TYPE_RGB24:
        #     return 3
        # elif mvImageType == KxMvImage.Type.MV_IMAGE_TYPE_RGB32:
        #     return 4
        # else:
        #     return 0
    
    def KxImage2PIL(self):
        #print 'KxImage2PIL', self.nWidth, self.nHeight, self.nChannel, self.nPitch, self.bAuto, len(self.buf)
        if self.nChannel == 1:
            pilimage = Image.frombuffer('L', (self.nWidth, self.nHeight), self.buf, 'raw', 'L', 0, 1)
        elif self.nChannel == 3:
            pilimage = Image.frombuffer('RGB', (self.nWidth, self.nHeight), self.buf, 'raw', 'RGB', 0, 1)
            r,g,b = pilimage.split()
            pilimage = Image.merge("RGB", (b, g, r))
        else:
            raise RuntimeError("KxImageBuffer's channel Error!self.nChannel:"+str(self.nChannel))
        return pilimage 
    
    
    def Kximage2npArr(self):
        if self.nChannel == 1:
            ModelImage = np.fromstring(self.buf, dtype=np.uint8).reshape(self.nHeight, self.nWidth)
        elif self.nChannel == 3:
            ModelImage = np.fromstring(self.buf, dtype=np.uint8).reshape(self.nHeight, self.nWidth, 3)
        else:
            raise RuntimeError("KxImageBuffer's channel Error!self.nChannel:"+str(self.nChannel))
        return ModelImage
    
     
    def PIL2KxImage(self, pilimage): 
        
        if pilimage.mode == 'L':       
           self.SetImageBuf(pilimage.tobytes(), pilimage.size[0], pilimage.size[1], pilimage.size[0] , 1, True)
        elif  pilimage.mode == 'RGB': 
           r,g,b = pilimage.split()
           pilimage = Image.merge("RGB", (b, g, r))  
           self.SetImageBuf(pilimage.tobytes(), pilimage.size[0], pilimage.size[1], pilimage.size[0] * 3, 3, True) 
        else:
            raise RuntimeError("PIL2KxImage Error! mode:"+str(pilimage.mode))
        
    def SaveBmp(self, path, isConvert = True):
        if self.nWidth != 0:
                    #print 'KxImage2PIL', self.nWidth, self.nHeight, self.nChannel, self.nPitch, self.bAuto, len(self.buf)
            if self.nChannel == 1:
                ModelImage = Image.frombuffer('L', (self.nWidth, self.nHeight), self.buf, 'raw', 'L', 0, 1)
            elif self.nChannel == 3:
                ModelImage = Image.frombuffer('RGB', (self.nWidth, self.nHeight), self.buf, 'raw', 'RGB', 0, 1)
                if isConvert == True:
                    r,g,b = ModelImage.split()
                    ModelImage = Image.merge("RGB", (b, g, r))
            else:
                raise RuntimeError("KxImageBuffer's channel Error!self.nChannel:"+str(self.nChannel))
            ModelImage.save(path) 
     
    def LoadBmp(self, path):
        if os.path.exists(path):
            ModelImage = Image.open(path)
            self.PIL2KxImage(ModelImage)
        else:
            raise RuntimeError("LoadBmp path Error!")
        
    def __ne__(self, other):
        self.notEqualInfStr = ''
        if type(self)!=type(other):
            self.notEqualInfStr += u'|The type of other is not (class KxImageBuf)|\r\n'
            return True
        
        if self.nWidth != other.nWidth:
            self.notEqualInfStr += u'|self.nWidth:'+str(self.nWidth) + u' other。nWidth:'+str(other.nWidth) + '|'
        
        if self.nHeight != other.nHeight:
            self.notEqualInfStr += u'|self.nHeight:'+str(self.nHeight) + u' other。nHeight:'+str(other.nHeight) + '|'
        
        if self.nPitch != other.nPitch:
            self.notEqualInfStr += u'|self.nPitch:'+str(self.nPitch) + u' other。nPitch:'+str(other.nPitch)+'|'
        
        if self.nChannel != other.nChannel:
            self.notEqualInfStr += u'|self.nChannel:'+str(self.nChannel) + u' other。nChannel:'+str(other.nChannel)+'|'
            
#         if self.bAuto != other.bAuto:
#             self.notEqualInfStr += u'|self.bAuto:'+str(self.bAuto) + u' other。bAuto:'+str(other.bAuto)+'|'
            
        if self.buf != other.buf:
            self.notEqualInfStr += u'|len(self.buf):'+str(len(self.buf)) + u' len(other。buf):'+str(len(other.buf))+'|'
                   
        if self.notEqualInfStr == '':
            return False
        else:
            self.notEqualInfStr += '\r\n'
            return True
