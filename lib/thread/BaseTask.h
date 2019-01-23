#pragma once

#include "BaseThread.h"

#include <functional>
#include <windows.h>

// TODO��
// ���������пռ�һ���̳߳ع���
// task��һ���Ѷ����߳�ʹ�������Լ������ͷŵ������Բ�Ҫ�ظ��ͷ�

enum TASK_CALLBACK_EV
{
	// ���õ�
	EV_BEGIN = 0,
	EV_SUCCESSED,
	EV_FAILED,

	// ConstructTask
	EV_FORMAT,
	EV_MODIFY_UEFI,
	EV_MODIFY_UD,
	EV_FORMAT_NEW,
	EV_MODIFY_DATA,
	EV_FORMAT_UPDATE,
	EV_SUCCESSED_UPDATE,

	// ����
	EV_DOWNLOAD_ALLSIZE,
	EV_DOWNLOAD_CURRENTSIZE,
	EV_DOWNLOAD_FAILURE,
	EV_DOWNLOAD_COMPLETE,

	// RestoreTask
	EV_RESTORE_UD_BEGIN,
	EV_FORMAT_PART_BEGIN,

	// CreateIsoTask
	EV_RESOURCE_RELEASE_BEGIN,
	EV_RESOURCE_RELEASE_PERCENT,
	EV_CREATE_ISO_BEGIN,

	// SimulateTask
	EV_LOADING_QEMU,

	// SysCheckerTask
	EV_GETCPTNAME,
	EV_GETOSINFO,
	EV_GETSYSROOTLETTER,
	EV_GETSYSROOTFILETYPE,
	EV_GETSYSROOTPARTIONTYPE,
	EV_GETBOOTTYPE,
	EV_GETCPUTYPE,
	EV_GETBASEBOARDINFO,
	EV_GETMEMORYCHIPSINFO,
	EV_GETROOTDISKINFO,
	EV_GETDISPLAYCARDINFO,

	// FileBackupTask
	EV_FILEALLCOUNT,
	EV_FILECURCOUNT,

	// DownTask
	EV_DOWN_START,
	EV_DOWN_COMPLETE,			// FALSE ʧ�� TRUE �ɹ�
	EV_DOWN_PROGRESS,

	// ö�ٵ���󣬳������ߵ������ֵ��Ϊ�ǳ���
	EV_ERROR,
	EV_COUNT = EV_ERROR
};

class BaseTask
{
public:
	virtual ~BaseTask();
	BaseTask()
	{

	}

public:
	void RegestCallBack(std::function<void(TASK_CALLBACK_EV, LPARAM)> callBack);
	virtual void DoTask() = 0;

protected:
	void Notify(TASK_CALLBACK_EV event, LPARAM lParam = 0);

protected:
	std::function<void(TASK_CALLBACK_EV, LPARAM)> m_notifyFun;
};
