#include "xx_signal.h"
#include "client.h"

int main() {
	// ���� SIGPIPE �źű�����Ϊ���ӹرճ���
	xx::IgnoreSignal();

	// ������ʵ��
	Client c;

	// ��ʼ����
	return c.Run();
}
