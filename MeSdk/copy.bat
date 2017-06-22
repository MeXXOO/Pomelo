::@echo on

md ..\sdk\include

::include file
copy .\include\platdefine.h	..\sdk\include\
copy .\include\include.h	..\sdk\include\

::base
copy .\include\array.h		..\sdk\include\
copy .\include\common.h		..\sdk\include\
copy .\include\data_pack.h	..\sdk\include\
copy .\include\event.h		..\sdk\include\
copy .\include\file.h		..\sdk\include\
copy .\include\list.h		..\sdk\include\
copy .\include\lock.h		..\sdk\include\
copy .\include\log.h		..\sdk\include\
copy .\include\map.h		..\sdk\include\
copy .\include\memory.h		..\sdk\include\
copy .\include\thread.h		..\sdk\include\

::socket
copy .\include\socket.h						..\sdk\include\
copy .\include\socket_addr.h				..\sdk\include\
copy .\include\socket_common.h				..\sdk\include\
copy .\include\socket_event.h				..\sdk\include\
copy .\include\socket_event_listener.h		..\sdk\include\
copy .\include\socket_listen.h				..\sdk\include\
copy .\include\socket_manager.h				..\sdk\include\
copy .\include\socket_manager_dispatch.h	..\sdk\include\
copy .\include\socket_tcp.h					..\sdk\include\
copy .\include\socket_udp.h					..\sdk\include\

::json
md ..\sdk\include\jsonc
copy .\jsonc\json_object.h					..\sdk\jsonc\
copy .\jsonc\json_tokener.h					..\sdk\jsonc\


::@echo off