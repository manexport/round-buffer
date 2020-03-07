/*
BSD 2-Clause License

Copyright (c) 2020, manexport<manexport@yeah.net>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>
#include "pthread.h"
#include "round_buffer.h"
#include "windows.h"
#include "mutipart_ data_parse.h"

#define READ_LENGTH  3200
typedef struct
{
	rb_t hd;
	char file_path[_MAX_PATH];
	char boundary[38];
}input_param;
void write_data_to_file(char* data, int len,const char* path)
{
	FILE* data_fd = NULL;
	data_fd = fopen(path, "ab+");
	if (data_fd)
	{
		fwrite(data, 1, len, data_fd);
		//fflush(data_fd);
	}
	fclose(data_fd);
	data_fd = NULL;
}
void* write_data_proc(void * input)
{
	//pthread_detach(pthread_self());
	char data_buf[READ_LENGTH];
	ullt read_len = READ_LENGTH;
	input_param *in = (input_param*)input;
	FILE* data_fd = fopen((char*)in->file_path, "rb");
	if (!data_fd) 
	{
		printf("open file failed!\n");
		return NULL;
	}
	while(1) 
	{
		read_len = fread(data_buf, 1, read_len, data_fd);
		if (read_len < READ_LENGTH)
		{

			round_buffer_write(in->hd,data_buf, read_len);
			set_shutdown(in->hd);
			fclose(data_fd);
			data_fd = NULL;
			break;
		}
		int n = round_buffer_write(in->hd,data_buf, read_len);
		printf("-----write %d\n", n);
		Sleep(10);
	}
	//pthread_exit(0);
	return NULL;
}
#define KEY_MAX_LENGTH (256)
#define BOUNDARY_LEN   (38)
typedef struct map_data_struct_s
{
	int stream_id;
	int shutdown;
	bool stop_signal;
	rb_t rb_hd;
	char key_string[KEY_MAX_LENGTH];
	char boundary[BOUNDARY_MAX_LENGTH];
	char file_path[_MAX_PATH];
} map_data_t;

void vsi_output_callback(int message, ullt fstParam, ullt sndParam, ullt thdParam)
{

}
void* http2_data_work_proc(void * input)
{
	char *input_data = NULL;
	map_data_t *in = (map_data_t*)input;
	mp_t mp_hd = (mp_t)mutipart_parse_init(vsi_output_callback);
	input_data = (char*)malloc(DATAUNPACK_READ_SIZE);
	if (!input_data)
		return NULL;
	char *next_read_pos = input_data;
	while (!in->stop_signal)
	{
		ullt size = get_used_size(in->rb_hd);
		if (size < sizeof(in->boundary))
		{
			Sleep(100);
			continue;
		}
		printf("----------------------------------------------we hv read %lld data from queue\n", size);
		/*calculate need data length */
		ullt need_read_len = DATAUNPACK_READ_SIZE - (next_read_pos - input_data);
		if ((need_read_len = (ullt)round_buffer_read(in->rb_hd, next_read_pos, need_read_len)) > 0)
		{
			/*get currently available data length */
			ullt available_len = (next_read_pos - input_data) + need_read_len;
			char *last_pos = NULL;
			if (strlen(in->boundary) == 0)
			{
				char* src_pos=mutipart_parse_strstr(mp_hd, input_data, available_len, MUTIPART_START);
				memcpy(in->boundary, src_pos+2, BOUNDARY_LEN);
			}
			mutipart_data_process(mp_hd, input_data, available_len,&last_pos,in->boundary,strlen(in->boundary));
			if (last_pos == input_data)
			{
				if (available_len >= DATAUNPACK_READ_SIZE)
				{
					next_read_pos = input_data;
				}
				else
					next_read_pos = input_data + available_len;
			}
			else if ((ullt)(last_pos - input_data) < available_len) 
			{
				char *input_data_new = (char*)malloc(DATAUNPACK_READ_SIZE);
				if (!input_data_new)
					break;
				int left_data_len = available_len - (last_pos - input_data);
				memcpy(input_data_new, last_pos, left_data_len * sizeof(char));
				if (input_data)
					free(input_data);
				input_data = input_data_new;
				next_read_pos = &input_data[left_data_len];
			}
			else
			{
				next_read_pos = input_data;
				Sleep(100);
			}
		}
		else
		{
			Sleep(100);
		}
	}
	if (input_data)
	{
		free(input_data);
		input_data = NULL;
	}
	return NULL;
}


int main()
{
	int ret = 0;
	input_param input1 = { {0} };
	char cur_path[_MAX_PATH] = { 0 };
	_getcwd(cur_path, _MAX_PATH);
	sprintf(input1.file_path, "%s\\Debug\\%s", cur_path, "output.dat");
	map_data_t data_item = { {0} };
	data_item.rb_hd= round_buffer_init(1024 * 1024 * 9);
	input1.hd = data_item.rb_hd;
	pthread_t * thread_data1 =(pthread_t *)calloc(1, sizeof(pthread_t));
	ret = pthread_create(thread_data1, NULL, write_data_proc, (void*)&input1);
	if (ret != 0)
	{
		return ret;
	}
	pthread_detach(*thread_data1);
	pthread_t * thread_data2 = (pthread_t *)calloc(1, sizeof(pthread_t));
	ret = pthread_create(thread_data2, NULL, http2_data_work_proc, (void*)&data_item);
	if (ret != 0)
	{
		return ret;
	}
	pthread_detach(*thread_data2);
	system("pause");
	round_buffer_destroy(data_item.rb_hd);
	return 0;
}


