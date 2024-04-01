# -*- coding: utf-8 -*-
#Used successfully in Python2.5 with matplotlib 0.91.2 and PyQt4 (andQt 4.3.3)
from distutils.core import setup
import py2exe
 
#We need to import the glob module to search for all files.
import glob
#import FileDialog 
#We need to exclude matplotlib backends not being used by thisexecutable. You may find
#that you need different excludes to create a working executable withyour chosen backend.
#We also need to include include various numerix libraries that theother functions call.
 
opts= {
'py2exe':{ "includes" : [ "sip","numpy",],
'excludes':['_gtkagg', '_tkagg', '_agg2', '_cairo', '_cocoaagg','_fltkagg','_gtk', '_gtkcairo'],
'dll_excludes':['libgdk-win32-2.0-0.dll','libgobject-2.0-0.dll','MSVCP90.dll'],
"packages":['FileDialog','scipy','skimage','libtiff','pywt',],
}
}
 
#Save matplotlib-data to mpl-data ( It is located in thematplotlib\mpl-data
#folder and the compiled programs will look for it in \mpl-data
#note: using matplotlib.get_mpldata_info
data_files= [(r'mpl-data',glob.glob(r'C:\Python27\Lib\site-packages\matplotlib\mpl-data\*.*')),
(r'mpl-data',glob.glob(r'C:\Python27\Lib\site-packages\zope\interface\*.*')),
(r'mpl-data',glob.glob(r'C:\Python27\Lib\site-packages\zope\interface\common\*.*')),
(r'mpl-data',glob.glob(r'C:\Python27\Lib\site-packages\zope\interface\tests\*.*')),
#Because matplotlibrc does not have an extension, glob does not findit (at least I think that's why)
#So add it manually here:
(r'mpl-data',[r'C:\Python27\Lib\site-packages\matplotlib\mpl-data\matplotlibrc']),
(r'mpl-data\images',glob.glob(r'C:\Python27\Lib\site-packages\matplotlib\mpl-data\images\*.*')),
(r'mpl-data\stylelib',glob.glob(r'C:\Python27\Lib\site-packages\matplotlib\style\*.*')),
(r'mpl-data\fonts',glob.glob(r'C:\Python27\Lib\site-packages\matplotlib\mpl-data\fonts\*.*'))]

#,"icon_resoureces": [(1, 'colorCard.png')] 
#"compressed": 1,
#"bundle_files": 1
#for console program use 'console = [{"script" :"scriptname.py"}]
# 注意：在压缩调试的时候，将windows改为console
setup(
    name = "KEXIN.",  
    version = "1.0", 
    description = "Card InSpector",
    # windows=[{"script": "main.py", "icon_resources":[(1, "res\log.ico")], "dest_base":"python"}],
    console=[{"script": "main.py",  "icon_resources":[(1, "res\log.ico")],"dest_base":"kexin"}],
    options=opts, 
    data_files=data_files)     
 
