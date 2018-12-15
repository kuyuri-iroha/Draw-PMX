#pragma once


template<class COMPointer>
void SafeRelease(COMPointer& _p)
{
	if (_p)
	{
		_p->Release();
		_p = nullptr;
	}
}

// åÎç∑íl
constexpr float EPSILON = 1e-8f;