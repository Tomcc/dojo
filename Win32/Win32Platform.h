#ifndef Win32Platform_h__
#define Win32Platform_h__

#include "dojo_common_header.h"

#include "Platform.h"

namespace Dojo
{
	class Win32Platform : public Platform
	{
	public:

		Win32Platform();

		virtual void initialise();
		virtual void shutdown();

		virtual void acquireContext();
		virtual void present();

		virtual std::string getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path );
		virtual void getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out );
		virtual uint loadFileContent( char*& bufptr, const std::string& path );
		virtual void loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height );

	protected:

		bool _hasExtension( const std::string& type, const std::string& nameOrPath );
	private:
	};
}

#endif // Win32Platform_h__