U
    Df�a�/  �                   @   s�   d dl mZ d dlmZmZmZ d dlmZ d dlm	Z	 d dl
mZ d dlmZ d dlT d dlT d dlmZ d d	lmZ d d
lmZ d dlmZ d dlZd dlZd dlZG dd� de�ZdS )�    )�KXBaseMainWidget)�QtGui�QtCore�	QtWidgets)�QWidget)�KxBaseRunLog)�KxBaseMonitoringWidget)�kxprivilege_management)�*)�ChipParameterSetting)�WorkListWidget)�DotCheckResultWidget)�ipc_toolNc                       s�   e Zd Z� fdd�Zdd� Zdd� Zdd� Z� fd	d
�Zdd� Zdd� Z	dd� Z
dd� Zdd� Zdd� Zdd� Zdd� Zdd� Zdd� Zdd � Zd$� fd"d#�	Z�  ZS )%�kxmainwindowc                    s�   t t| ��|� tj|d | d�| _t| |d�| _t| �| _	t
� | _t| �| _tjddd�| _| �| jj| jj| jj| jjg| j| j| j	| jg� | ��  | ��  | jj�d� d S )NZmointoringwidget_classname)�nameZh_parent)Zhparent�dict_configZCOM3i � )�portZbaudrateu   下箱体托盘检测)�superr   �__init__r   �create�widget_Realtimer   �widget_Paramsettingr   Zwidget_runlogr	   �widget_permissionr   Zwidge_worklist�serialZSerial�mySeriaZ_initstackwidget�uiZpbt_realtimeZpbt_paramsetZpbt_logviewZpushButton_worklist�_completeui�_completeconnectZlabel_2�setText)�selfr   ��	__class__� �IG:\ZS\project\nd-gulecheck\mainstation\project\mainwindow\kxmainwindow.pyr      s    

�zkxmainwindow.__init__c                 C   s�  t �| �| _| j�t�dd�� | j�t�dd�� | j�t�	d�� | j�
d� | j�t�dd�� | j�tjj� | jj�| j� t �| �| _| j�t�dd�� | j�t�dd�� | j�t�	d�� | j�
d� | j�t�dd�� | j�tjj� | jj�| j� t�d�| _t �| jj�| _| j�tjj� | j�d� | j�| j� | j�| j� t�� }|�d� |�d	� | jj �!|� d S )
N�d   u   res/设备点检.png�color:white;border:none;�Z   u   res/设备自启测试.pngzres\MES.png�F   ZArial�   )"r   �QToolButton�toolbutton_move�setMinimumSizer   �QSize�setMaximumSize�setIconr   �QIcon�setStyleSheet�setIconSizeZsetToolButtonStyle�Qt�ToolButtonTextUnderIconr   ZverticalLayout_2�	addWidget�toolbutton_test�QPixmapZQPixmap_disMES�QLabelZlabelwidgetZ	label_MES�setAlignment�AlignCenter�setFixedWidth�	setPixmapZ	statusBar�QFontZ	setFamily�setPointSize�toolButton_userlevel�setFont)r   �fontr"   r"   r#   r   %   s4    

zkxmainwindow._completeuic                 C   s6   | j jj�| j� | jj�| j� | jj�| j� d S �N)	r   r>   �clicked�connect�showpermissiondialogr*   �_ready2dotcheckr5   �_ready2show_machine_move�r   r"   r"   r#   r   G   s    zkxmainwindow._completeconnectc                 C   s6   | j j�� r | j�dddd� n| j�dddd� d S )Nr   �   ZsetlearnstatusTF)r   Ztoolbtn_learn�	isCheckedr   Zstr2paramitemfunrG   r"   r"   r#   �_setlearnstatusM   s    zkxmainwindow._setlearnstatusc                    s(   t t| ���  | jj�� r$| j��  d S rA   )r   r   �_offlinerunr   Ztoolbtn_offlinerunrI   r   �clearrG   r    r"   r#   rK   S   s    zkxmainwindow._offlinerunc                 C   s.   | j �d� | j ��  | j �� }| �|� d S )Nzd:\)r   Zsetpasswordpath�exec_Zgetpermissionlevel�updatepermission)r   Zpermissonlevelr"   r"   r#   rD   X   s    

z!kxmainwindow.showpermissiondialogc                 C   sh   | j j�d� |dkr&| j j�d� n>|dkr>| j j�d� n&|dkrV| j j�d� n| j j�d� d S )	Nr%   r   u	   操作员rH   ZME�   ZIMDu	   管理员)r   r>   r0   r   )r   ZPERMISSIONLEVELr"   r"   r#   rN   ^   s    zkxmainwindow.updatepermissionc                 C   s   t j| jd�}|��  dS )u   启动点检或master��targetN)�	threading�Thread�_control_calibrate�start�r   �tr"   r"   r#   rE   j   s    zkxmainwindow._ready2dotcheckc                 C   s   t j| jd�}|��  dS )u   启动设备rP   N)rR   rS   �_controlmachinerU   rV   r"   r"   r#   rF   q   s    z%kxmainwindow._ready2show_machine_movec                 C   sP   t |�}|d }t |d �}t |d �}|d }t |d �}|d }||||gS )u�   
            将移动距离转换为hex ->  转换为两个十六进制
            例如0x27eab: VALUE1为0x7e,VALUE2为0xab,VALUE3为0x00,VA