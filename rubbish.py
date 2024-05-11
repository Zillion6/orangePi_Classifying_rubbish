# -*- coding: utf-8 -*-
# ����������
# pip install alibabacloud_imagerecog20190930

import os
import io
from urllib.request import urlopen
from alibabacloud_imagerecog20190930.client import Client
from alibabacloud_imagerecog20190930.models import ClassifyingRubbishAdvanceRequest
from alibabacloud_tea_openapi.models import Config
from alibabacloud_tea_util.models import RuntimeOptions

config = Config(
    # ����AccessKey ID��AccessKey Secret����ο�https://help.aliyun.com/document_detail/175144.html��
    # ������õ���RAM�û���AccessKey������ҪΪRAM�û�����Ȩ��AliyunVIAPIFullAccess����ο�https://help.aliyun.com/document_detail/145025.html
    # �ӻ���������ȡ���õ�AccessKey ID��AccessKey Secret�����д���ʾ��ǰ���������û���������
    access_key_id=os.environ.get('ALIBABA_CLOUD_ACCESS_KEY_ID'),
    access_key_secret=os.environ.get('ALIBABA_CLOUD_ACCESS_KEY_SECRET'),
    # ���ʵ�����
    endpoint='imagerecog.cn-shanghai.aliyuncs.com',
    # ���ʵ�������Ӧ��region
    region_id='cn-shanghai'
)

def alicloud_classify_Rubbish():
    #����һ���ļ��ڱ���
    img = open(r'/home/orangepi/Project/rubbish/rubbish.jpg', 'rb')
    #��������ʹ������ɷ��ʵ�url
    #url = 'https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/viapi-3.0domepic/imagerecog/ClassifyingRubbish/ClassifyingRubbish1.jpg'
    #img = io.BytesIO(urlopen(url).read())
    classifying_rubbish_request = ClassifyingRubbishAdvanceRequest()
    classifying_rubbish_request.image_urlobject = img
    runtime = RuntimeOptions()
    try:
       # ��ʼ��Client
       client = Client(config)
       response = client.classifying_rubbish_advance(classifying_rubbish_request, runtime)
       # ��ȡ������
       print(response.body)
       return response.body.to_map()['Data']['Elements'][0]['Category']
    except Exception as error:
       # ��ȡ���屨����Ϣ
       print(error)
       # ��ȡ�����ֶ�
       print(error.code)
    
if __name__ == "__main__":
    alicloud_classify_Rubbish()


# import os
# import io
# import json
# from urllib.request import urlopen
# from alibabacloud_imagerecog20190930.client import Client
# from alibabacloud_imagerecog20190930.models import ClassifyingRubbishAdvanceRequest
# from alibabacloud_tea_openapi.models import Config
# from alibabacloud_tea_util.models import RuntimeOptions

# config = Config(
#     # 创建AccessKey ID和AccessKey Secret，请参考
#     #https://help.aliyun.com/document_detail/175144.html。
#     # 如果您用的是RAM用户的AccessKey，还需要为RAM用户授予权限AliyunVIAPIFullAccess，请参考
#     #https://help.aliyun.com/document_detail/145025.html
#     # 从环境变量读取配置的AccessKey ID和AccessKey Secret。运行代码示例前必须先配置环境变量。
#     access_key_id=os.environ.get('ALIBABA_CLOUD_ACCESS_KEY_ID'),
#     access_key_secret=os.environ.get('ALIBABA_CLOUD_ACCESS_KEY_SECRET'),
#     # 访问的域名
#     endpoint='imagerecog.cn-shanghai.aliyuncs.com',
#     # 访问的域名对应的region
#     region_id='cn-shanghai'
# )

# def alibabacloud_garbage():
#     img = open(r'/home/orangepi/Project/rubbish/rubbish.jpg', 'rb')
#     #场景二：使用任意可访问的url
#     #url = 'https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/viapi3.0domepic/imagerecog/ClassifyingRubbish/ClassifyingRubbish1.jpg'
#     #img = io.BytesIO(urlopen(url).read())
#     classifying_rubbish_request = ClassifyingRubbishAdvanceRequest()
#     classifying_rubbish_request.image_urlobject = img
#     runtime = RuntimeOptions()
#     try:
#         # 初始化Client
#         client = Client(config)
#         response = client.classifying_rubbish_advance(classifying_rubbish_request, runtime)
#         print(response.body)
#         return response.body.to_map()['Data']['Elements'][0]['Category']
#     except Exception as error:
#         print(type('获取失败'))
#     return '获取失败'
