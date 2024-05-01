#ifndef __ENGINE_INITIALIZER__
#define __ENGINE_INITIALIZER__

struct ptrClass;
struct GraphicsDataMisc;

namespace Ge
{
    class Initializer
    {
	protected:
            virtual bool initialize() = 0;
            virtual void release() = 0;
    };

	class InitializerAPI
	{
	protected:
		virtual bool initialize(GraphicsDataMisc * gdm) = 0;
		virtual void release() = 0;
	};

	class InitializerPtr
	{
	protected:
		virtual bool initialize(ptrClass * ptrc) = 0;
		virtual void release() = 0;
	};
}

#endif //!__ENGINE_INITIALIZER__