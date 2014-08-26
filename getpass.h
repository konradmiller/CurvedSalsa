#ifndef GETPASS_H_
#define GETPASS_H_

#ifdef __ELF__
	#include <unistd.h>
#endif


#ifdef _MSC_VER
	#include <conio.h>

	#ifndef PASS_MAX
		#define PASS_MAX 512
	#endif

	char* getpass ( const char *prompt )
	{
		char getpassbuf[PASS_MAX + 1];
		size_t i = 0;
		int c;

		if( prompt )
		{
			fputs( prompt, stderr );
			fflush( stderr );
		}

		for( ;; )
		{
			c = _getch();

			if( c == '\r' )
			{
				getpassbuf[i] = '\0';
				break;
			}
			else if( i < PASS_MAX )
			{
				getpassbuf[i++] = c;
			}

			if( i >= PASS_MAX )
			{
				getpassbuf[i] = '\0';
				break;
			}
		}

		if( prompt )
		{
			fputs( "\r\n", stderr );
			fflush( stderr );
		}

		return _strdup( getpassbuf );
	}
#endif // _MSC_VER

#endif // GETPASS_H_
