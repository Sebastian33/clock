#include <time.h>
typedef unsigned char u8;

class RTCDriver
{
public:
	void Init();
	void WriteDateTime(const tm& datetime);
	tm ReadDateTime();
	
private:
	void Write(unsigned address, unsigned offset, u8* buf, unsigned size);
	void Read(unsigned address, unsigned offset, u8* buf, unsigned size);
};