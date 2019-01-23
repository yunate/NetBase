#ifndef __DownloaderHelper_H_
#define __DownloaderHelper_H_

#include "Downloader.h"
#include "DownloaderPool.h"
#include "Downloader.h"

#include <string>

enum DOWNINFOSTATE
{
	DOWNINFOSTATE_ALLSIZE,
	DOWNINFOSTATE_CURSIZE,
	DOWNINFOSTATE_FAILURE,
	DOWNINFOSTATE_SUCCESS
};

class DownloadHelper
{
public:
	/*
		Create Execute Clean
		��������ͨ�����غ���
		��Create��������ʼ����һ������
		��Execute��ʼִ��һ�����أ��������߳��
		��Clean���ؽ�����������Դ
	*/ 
	static void *Create(const RequestInfo&);
	static void Execute(void * handle);
	static void Clean(void * handle);

	/*
		ͨ����������ѭ���������ݣ���ÿһ��nType���ͽ��д���������Ҫ��������дһ�� 
		@param : nType ����
		@param : callback һ��ͨ�ûص�
		@return : ����true˵�����������ܳɹ�Ҳ����ʧ�ܣ���falseû����
	*/ 
	static bool DownDataPeek_Core(DOWNLOADER_TYPE nType, std::function<void(DOWNINFOSTATE, unsigned int)> callback);
	static bool DownDataPeek_Core(void * handle, std::function<void(DOWNINFOSTATE, unsigned int)> callback);

	// �����ļ���������
	static void * DownLoadWithMD5Check(const RequestInfo&);

	static void * DownLoadWithFileNameCheck(const RequestInfo&);
};

#endif