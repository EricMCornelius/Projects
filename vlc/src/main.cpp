#include <stdio.h>
#include <windows.h>
#include <vlc/vlc.h>
#include <iostream>
#include <fstream>

std::ofstream log("C:/Projects/vlc/log.txt");

void checkErr() {
	const char* err = libvlc_errmsg();
    if (err) {
       log << err << std::endl;
       exit(-1);
    }
}

int main(int argc, char **argv) {
   libvlc_instance_t *inst;
   libvlc_media_discoverer_t *discoverer;
   libvlc_media_list_t *list;

   inst = libvlc_new(argc, argv);
   if (!inst)
	  log << "Error" << std::endl;
   checkErr();
   discoverer = libvlc_media_discoverer_new_from_name(inst, "dshow2");
   checkErr();
   list = libvlc_media_discoverer_media_list(discoverer);
   checkErr();
   int count = libvlc_media_list_count(list);
   log << "Count: " << count << std::endl;
   //Sleep (10000);
   libvlc_release(inst);
   return 0;
}