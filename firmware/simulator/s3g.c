#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>

#include "Simulator.hh"
#include "Commands.hh"
#include "s3g_private.h"
#include "s3g_stdio.h"
#include "s3g.h"

typedef struct {
     union {
	  int32_t  i;
	  uint32_t u;
	  float    f;
	  unsigned char c[4];
     } u;
} foo_32_t;

typedef struct {
     union {
	  int16_t  i;
	  uint16_t u;
	  unsigned char c[2];
     } u;
} foo_16_t;

typedef struct {
     uint8_t     cmd_id;
     size_t      cmd_len;
     int         cmd_blocking;
     const char *cmd_desc;
} s3g_command_info_t;

static const s3g_command_info_t command_table_raw[] = {
     /*   0 */  {HOST_CMD_VERSION, 0, 0, "version"},
     /*   1 */  {HOST_CMD_INIT, 0, -1, "initialize"},
     /*   2 */  {HOST_CMD_GET_BUFFER_SIZE, 0, 0, "get buffer size"},
     /*   3 */  {HOST_CMD_CLEAR_BUFFER, 0, 0, "clear buffer"},
     /*   4 */  {HOST_CMD_GET_POSITION, 0, -1, "get position"},
     /*   7 */  {HOST_CMD_ABORT, 0, -1, "abort"},
     /*   8 */  {HOST_CMD_PAUSE, 0, -1, "pause"},
     /*   9 */  {HOST_CMD_PROBE, 0, -1, "probe"},
     /*  10 */  {HOST_CMD_TOOL_QUERY, 0, 0, "tool query"},
     /*  11 */  {HOST_CMD_IS_FINISHED, 0, -1, "is finished?"},
     /*  12 */  {HOST_CMD_READ_EEPROM, 0, 0, "read EEPROM"},
     /*  13 */  {HOST_CMD_WRITE_EEPROM, 0, 0, "write EEPROM"},
     /*  14 */  {HOST_CMD_CAPTURE_TO_FILE, 0, -1, "capture to file"},
     /*  15 */  {HOST_CMD_END_CAPTURE, 0, -1, "end capture"},
     /*  16 */  {HOST_CMD_PLAYBACK_CAPTURE, 0, -1, "playback capture"},
     /*  17 */  {HOST_CMD_RESET, 0, -1, "software reset"},
     /*  18 */  {HOST_CMD_NEXT_FILENAME, 0, -1, "next SD card filename"},
     /*  19 */  {HOST_CMD_GET_DBG_REG, 0, 0, "get debug register"},
     /*  20 */  {HOST_CMD_GET_BUILD_NAME, 0, 0, "get build name"},
     /*  21 */  {HOST_CMD_GET_POSITION_EXT, 0, -1, "get position extended"},
     /*  22 */  {HOST_CMD_EXTENDED_STOP, 0, -1, "extended stop"},
     /*  23 */  {HOST_CMD_BOARD_STATUS, 0, 0, "get board status"},
     /*  24 */  {HOST_CMD_GET_BUILD_STATS, 0, -1, "get build statistics"},
     /*  27 */  {HOST_CMD_ADVANCED_VERSION, 0, 0, "advanced version"},
     /* 112 */  {HOST_CMD_DEBUG_ECHO, 0, -1, "debug echo"},
     /* 130 */
     /* 131 */  {HOST_CMD_FIND_AXES_MINIMUM, 7, -1, "find axes minimum"},
     /* 132 */  {HOST_CMD_FIND_AXES_MAXIMUM, 7, -1, "find axes maximum"},
     /* 133 */  {HOST_CMD_DELAY, 4, -1, "delay"},
     /* 134 */  {HOST_CMD_CHANGE_TOOL, 1, -1, "change tool"},
     /* 135 */  {HOST_CMD_WAIT_FOR_TOOL, 5, -1, "wait for tool ready"},
     /* 136 */  {HOST_CMD_TOOL_COMMAND, 0xffffffff, 0, "tool action"},
     /* 137 */  {HOST_CMD_ENABLE_AXES, 1, -1, "enable/disable axes"},
     /* 138 */  {138, 2, -1, "user block"},
     /* 139 */  {HOST_CMD_QUEUE_POINT_EXT, 24, -1, "queue point extended"},
     /* 140 */  {HOST_CMD_SET_POSITION_EXT, 20, -1, "set position extended"},
     /* 141 */  {HOST_CMD_WAIT_FOR_PLATFORM, 5, -1, "wait for platform ready"},
     /* 142 */  {HOST_CMD_QUEUE_POINT_NEW, 25, -1, "queue new point"},
     /* 143 */  {HOST_CMD_STORE_HOME_POSITION, 1, -1, "store home position"},
     /* 144 */  {HOST_CMD_RECALL_HOME_POSITION, 1, -1, "recall home position"},
     /* 145 */  {HOST_CMD_SET_POT_VALUE, 2, -1, "digital potentiometer"},
     /* 146 */  {HOST_CMD_SET_RGB_LED, 5, -1, "RGB LED"},
     /* 147 */  {HOST_CMD_SET_BEEP, 5, -1, "buzzer beep"},
     /* 148 */  {HOST_CMD_PAUSE_FOR_BUTTON, 4, -1, "pause for button"},
     /* 149 */  {HOST_CMD_DISPLAY_MESSAGE, -1, -1, "display message"},
     /* 150 */  {HOST_CMD_SET_BUILD_PERCENT, 2, 0, "build percentage"},
     /* 151 */  {HOST_CMD_QUEUE_SONG, 1, -1, "queue song"},
     /* 152 */  {HOST_CMD_RESET_TO_FACTORY, 1, -1, "restore to factory settings"},
     /* 153 */  {HOST_CMD_BUILD_START_NOTIFICATION, 4, -1, "build start notification"},
     /* 154 */  {HOST_CMD_BUILD_END_NOTIFICATION, 1, -1, "build end notification"},
     /* 155 */  {HOST_CMD_QUEUE_POINT_NEW_EXT, 31, 0, "queue point new extended"},
     /* 156 */  {HOST_CMD_SET_ACCELERATION_TOGGLE, 1, -1, "set segment acceleration"},
     /* 157 */  {HOST_CMD_STREAM_VERSION, 20, 0, "stream version"},
     /* 158 */  {HOST_CMD_PAUSE_AT_ZPOS, 4, 0, "pause at Z position"}
};

static const s3g_command_info_t tool_command_table_raw[] = {
     /*   0 */  {SLAVE_CMD_VERSION, 0, 0, "version"},
     /*   1 */  {SLAVE_CMD_INIT, 0, -1, "initialize"},
     /*   2 */  {SLAVE_CMD_GET_TEMP, 0, 0, "query current extruder temperature"},
     /*   3 */  {SLAVE_CMD_SET_TEMP, 0, 0, "set extruder target temperature"},
     /*   4 */  {SLAVE_CMD_SET_MOTOR_1_PWM, 0, -1, "set motor 1 speed (PWM)"},
     /*   5 */  {SLAVE_CMD_SET_MOTOR_2_PWM, 0, -1, "set motor 2 speed (PWM)"},
     /*   6 */  {SLAVE_CMD_SET_MOTOR_1_RPM, 0, -1, "set motor 1 speed (RPM)"},
     /*   7 */  {SLAVE_CMD_SET_MOTOR_2_RPM, 0, -1, "set motor 2 speed (RPM)"},
     /*   8 */  {SLAVE_CMD_SET_MOTOR_1_DIR, 0, -1, "set motor 1 direction"},
     /*   9 */  {SLAVE_CMD_SET_MOTOR_2_DIR, 0, -1, "set motor 2 direction"},
     /*  10 */  {SLAVE_CMD_TOGGLE_MOTOR_1, 0, -1, "set motor 1 state"},
     /*  11 */  {SLAVE_CMD_TOGGLE_MOTOR_2, 0, -1, "set motor 2 state"},
     /*  12 */  {SLAVE_CMD_TOGGLE_FAN, 0, 0, "set heatsink cooling fan state"},
     /*  13 */  {SLAVE_CMD_TOGGLE_VALVE, 0, 0, "set print cooling fan state"},
     /*  14 */  {SLAVE_CMD_SET_SERVO_1_POS, 0, -1, "set servo 1 position"},
     /*  15 */  {SLAVE_CMD_SET_SERVO_2_POS, 0, -1, "set servo 2 position"},
     /*  16 */  {SLAVE_CMD_FILAMENT_STATUS, 0, 0, "query filament status"},
     /*  17 */  {SLAVE_CMD_GET_MOTOR_1_RPM, 0, 0, "query motor 1 speed (RPM)"},
     /*  18 */  {SLAVE_CMD_GET_MOTOR_2_RPM, 0, 0, "query motor 2 speed (RPM)"},
     /*  19 */  {SLAVE_CMD_GET_MOTOR_1_PWM, 0, 0, "query motor 1 speed (PWM)"},
     /*  20 */  {SLAVE_CMD_GET_MOTOR_2_PWM, 0, 0, "query motor 2 speed (PWM)"},
     /*  21 */  {SLAVE_CMD_SELECT_TOOL, 0, -1, "switch tool"},
     /*  22 */  {SLAVE_CMD_IS_TOOL_READY, 0, -1, "query tool ready"},
     /*  23 */  {SLAVE_CMD_PAUSE_UNPAUSE, 0, -1, "toggle pause state"},
     /*  24 */  {SLAVE_CMD_ABORT, 0, -1, "abort"},
     /*  25 */  {SLAVE_CMD_READ_FROM_EEPROM, 0, 0, "read EEPROM"},
     /*  26 */  {SLAVE_CMD_WRITE_TO_EEPROM, 0, 0, "write EEPROM"},
     /*  30 */  {SLAVE_CMD_GET_PLATFORM_TEMP, 0, 0, "query current platform temperature"},
     /*  31 */  {SLAVE_CMD_SET_PLATFORM_TEMP, 0, 0, "set platform target temperature"},
     /*  32 */  {SLAVE_CMD_GET_SP, 0, 0, "query extruder target temperature"},
     /*  33 */  {SLAVE_CMD_GET_PLATFORM_SP, 0, 0, "query platform target temperature"},
     /*  34 */  {SLAVE_CMD_GET_BUILD_NAME, 0, 0, "query build name"},
     /*  35 */  {SLAVE_CMD_IS_PLATFORM_READY, 0, -1, "query platform ready"},
     /*  36 */  {SLAVE_CMD_GET_TOOL_STATUS, 0, 0, "query tool status"},
     /*  37 */  {SLAVE_CMD_GET_PID_STATE, 0, 0, "query PID state"},
     /*  40 */  {SLAVE_CMD_LIGHT_INDICATOR_LED, 0, -1, "set LED state"}
};

static s3g_command_info_t command_table[256];
static s3g_command_info_t tool_command_table[256];

// Not thread safe

static int tables_initialized = 0;

static int s3g_init(void)
{
     int i, istat;
     const s3g_command_info_t *p;

     if (tables_initialized != 0)
	  return(0);

     // Initialize the indexed command tables
     memset(command_table,      0, sizeof(command_table));
     memset(tool_command_table, 0, sizeof(tool_command_table));

     // Force all unrecognized commands to be blocking
     for (i = 0; i < 256; i++)
     {
	  command_table[i].cmd_blocking      = -1;
	  tool_command_table[i].cmd_blocking = -1;
     }

     istat = 0;
     p = command_table_raw;

     // Load the indexed command table, looking for conflicts
     for (i = 0; i < sizeof(command_table_raw) / sizeof(s3g_command_info_t); i++, p++)
     {
	  if ((command_table[p->cmd_id].cmd_len != 0 ||
	       command_table[p->cmd_id].cmd_desc != NULL))
	  {
	       // Table already has an entry for this command id
	       // Make sure that the lengths don't conflict
	       if (command_table[p->cmd_id].cmd_len != p->cmd_len)
	       {
		    fprintf(stderr,
			    "s3g_init(%d): Two commands with identical ids (%d) but "
			    "different lengths encountered; ignoring \"%s\"\n",
			    __LINE__, p->cmd_id, p->cmd_desc ? p->cmd_desc : "<no name>");
		    istat = -1;
		    continue;
	       }
	  }
	  command_table[p->cmd_id].cmd_id       = p->cmd_id;
	  command_table[p->cmd_id].cmd_len      = p->cmd_len;
	  command_table[p->cmd_id].cmd_desc     = p->cmd_desc;
	  command_table[p->cmd_id].cmd_blocking = p->cmd_blocking;
     }

     p = tool_command_table_raw;

     // Load the indexed command table, looking for conflicts
     for (i = 0; i < sizeof(tool_command_table_raw) / sizeof(s3g_command_info_t); i++, p++)
     {
	  if ((tool_command_table[p->cmd_id].cmd_len != 0 ||
	       tool_command_table[p->cmd_id].cmd_desc != NULL))
	  {
	       // Table already has an entry for this command id
	       // Make sure that the lengths don't conflict
	       if (tool_command_table[p->cmd_id].cmd_len != p->cmd_len)
	       {
		    fprintf(stderr,
			    "s3g_init(%d): Two tool commands with identical ids (%d) but "
			    "different lengths encountered; ignoring \"%s\"\n",
			    __LINE__, p->cmd_id, p->cmd_desc ? p->cmd_desc : "<no description>");
		    istat = -1;
		    continue;
	       }
	  }
	  tool_command_table[p->cmd_id].cmd_id       = p->cmd_id;
	  tool_command_table[p->cmd_id].cmd_len      = p->cmd_len;
	  tool_command_table[p->cmd_id].cmd_desc     = p->cmd_desc;
	  tool_command_table[p->cmd_id].cmd_blocking = p->cmd_blocking;
     }

     tables_initialized = -1;

     return(istat);
}


s3g_context_t *s3g_open(int type, void *src, int flags, int mode)
{
     s3g_context_t *ctx;

     (void)type; // Only type is a file

     ctx = (s3g_context_t *)calloc(1, sizeof(s3g_context_t));
     if (!ctx)
     {
	  fprintf(stderr, "s3g_open(%d): Unable to allocate VM; %s (%d)\n",
		  __LINE__, strerror(errno), errno);
	  return(NULL);
     }

     if (s3g_stdio_open(ctx, src, flags, mode))
	  return(NULL);

     return(ctx);
}


int s3g_close(s3g_context_t *ctx)
{
     int iret;

     if (!ctx)
	  return(0);

     iret = (ctx->close != NULL) ? (*ctx->close)(ctx->r_ctx) : 0;

     free(ctx);

     return(iret);
}

int s3g_command_isblocking(s3g_command_t *cmd)
{
     if (!cmd)
	  // Bad call: claim the worst case which is blocking true;
	  return(-1);

     if (cmd->cmd_id != HOST_CMD_TOOL_COMMAND)
	  return (command_table[cmd->cmd_id].cmd_blocking);
     else
	  return(tool_command_table[cmd->t.tool.subcmd_id].cmd_blocking);
	       
}

int s3g_command_read_ext(s3g_context_t *ctx, s3g_command_t *cmd,
			 unsigned char *buf, size_t maxbuf, size_t *buflen)
{
     unsigned char *buf0 = buf;
     ssize_t bytes_expected, bytes_read;
     s3g_command_info_t *ct;
     s3g_command_t dummy;
     foo_16_t f16;
     foo_32_t f32;
     int iret;
     uint8_t ui8arg;

     iret = -1;

     if (buflen)
	  *buflen = 0;

     // We have to have a read context
     // We don't need a command context to return the command in
     if (!ctx || !buf || maxbuf == 0)
     {
	  fprintf(stderr, "s3g_command_get(%d): Invalid call; ctx=%p, buf=%p, "
		  "maxbuf=%lu\n", __LINE__, (void *)ctx, (void *)buf, maxbuf);
	  errno = EINVAL;
	  return(-1);
     }
     else if (!ctx->read)
     {
	  fprintf(stderr, "s3g_command_get(%d): Invalid context; "
		  "ctx->read=NULL\n", __LINE__);
	  errno = EINVAL;
	  return(-1);
     }
     else if (!buf || maxbuf == 0)
     {
	  fprintf(stderr, "s3g_command_get(%d): Invalid context; "
		  "ctx->read=NULL\n", __LINE__);
	  errno = EINVAL;
	  return(-1);
     }

     // Initialize command table
     s3g_init();

     if (1 != (bytes_expected = (*ctx->read)(ctx->r_ctx, buf0, maxbuf, 1)))
     {
	  // End of file condition?
	  if (bytes_expected == 0)
	       return(1); // EOF

	  fprintf(stderr,
		  "s3g_command_get(%d): Error while reading from the s3g file; "
		  "%s (%d)\n",
		  __LINE__, strerror(errno), errno);
	  return(-1);
     }

     ct = command_table + buf0[0];  // &command_table[buf0[0]]

     buf    += 1;
     maxbuf -= 1;

     if (!cmd)
	  cmd = &dummy;

     cmd->cmd_id      = ct->cmd_id;
     cmd->cmd_desc    = ct->cmd_desc;
     cmd->cmd_len     = ct->cmd_len;
     cmd->cmd_raw_len = 0;

     if (ct->cmd_desc == NULL)
     {
	  fprintf(stderr,
		  "s3g_command_get(%d): Unrecognized command, %d\n",
		  __LINE__, buf0[0]);
	  goto done;
     }

#define GET_INT32(v) \
	  if (maxbuf < 4) goto trunc; \
	  if (4 != (bytes_read = (*ctx->read)(ctx->r_ctx, buf, maxbuf, 4))) \
	       goto io_error; \
	  memcpy(&f32.u.c, buf, 4); \
	  buf    += bytes_read; \
	  maxbuf -= bytes_read; \
	  cmd->t.v = f32.u.i

#define GET_UINT32(v) \
	  if (maxbuf < 4) goto trunc; \
	  if (4 != (bytes_read = (*ctx->read)(ctx->r_ctx, buf, maxbuf, 4))) \
	       goto io_error; \
	  memcpy(&f32.u.c, buf, 4); \
	  buf    += bytes_read; \
	  maxbuf -= bytes_read; \
	  cmd->t.v = f32.u.u

#define GET_FLOAT32(v) \
	  if (maxbuf < 4) goto trunc; \
	  if (4 != (bytes_read = (*ctx->read)(ctx->r_ctx, buf, maxbuf, 4))) \
	       goto io_error; \
	  memcpy(&f32.u.c, buf, 4); \
	  buf    += bytes_read; \
	  maxbuf -= bytes_read; \
	  cmd->t.v = f32.u.f;

#define GET_UINT8(v) \
	  if (maxbuf < 1) goto trunc; \
	  if (1 != (bytes_read = (*ctx->read)(ctx->r_ctx, buf, maxbuf, 1))) \
	       goto io_error; \
	  ui8arg = buf[0]; \
	  buf    += bytes_read; \
	  maxbuf -= bytes_read; \
	  cmd->t.v = ui8arg

#define GET_INT16(v) \
	  if (maxbuf < 2) goto trunc; \
	  if (2 != (bytes_read = (*ctx->read)(ctx->r_ctx, buf, maxbuf, 2))) \
	       goto io_error; \
	  memcpy(&f16.u.c, buf, 2); \
	  buf    += bytes_read; \
	  maxbuf -= bytes_read; \
	  cmd->t.v = f16.u.i

#define GET_UINT16(v) \
	  if (maxbuf < 2) goto trunc; \
	  if (2 != (bytes_read = (*ctx->read)(ctx->r_ctx, buf, maxbuf, 2))) \
	       goto io_error; \
	  memcpy(&f16.u.c, buf, 2); \
	  buf    += bytes_read; \
	  maxbuf -= bytes_read; \
	  cmd->t.v = f16.u.u

#define ZERO(v,c) cmd->t.v = (c)0

     switch(cmd->cmd_id)
     {
     case HOST_CMD_DELAY :
	  GET_UINT32(delay.millis);
	  break;

     case HOST_CMD_FIND_AXES_MINIMUM :
     case HOST_CMD_FIND_AXES_MAXIMUM :
	  GET_UINT8(find_axes_minmax.flags);
	  GET_UINT32(find_axes_minmax.feedrate);
	  GET_UINT16(find_axes_minmax.timeout);
	  break;

     case HOST_CMD_WAIT_FOR_TOOL :
	  GET_UINT8(wait_for_tool.index);
	  GET_UINT16(wait_for_tool.ping_delay);
	  GET_UINT16(wait_for_tool.timeout);
	  break;

     case HOST_CMD_WAIT_FOR_PLATFORM :
	  GET_UINT8(wait_for_platform.index);
	  GET_UINT16(wait_for_platform.ping_delay);
	  GET_UINT16(wait_for_platform.timeout);
	  break;

     case HOST_CMD_STORE_HOME_POSITION :
	  GET_UINT8(store_home_position.axes);
	  break;

     case HOST_CMD_RECALL_HOME_POSITION :
	  GET_UINT8(recall_home_position.axes);
	  break;

     default :
	  // Just read the data
	  bytes_expected = (ssize_t)(ct->cmd_len & 0x7fffffff);
	  if ((bytes_read = (*ctx->read)(ctx->r_ctx, buf, maxbuf,
					 ct->cmd_len)) != bytes_expected)
	       goto io_error;

	  buf    += bytes_read;
	  maxbuf -= bytes_read;
	  break;

     case HOST_CMD_TOOL_COMMAND :
	  // This command is VERY MBI specific
	  if ((ssize_t)3 != (*ctx->read)(ctx->r_ctx, buf, maxbuf, 3))
	       goto io_error;
	  if (cmd)
	       cmd->cmd_len = (size_t)buf[2];
	  cmd->t.tool.subcmd_id  = buf[1];
	  cmd->t.tool.index      = buf[0];
	  cmd->t.tool.subcmd_len = bytes_expected = (ssize_t)buf[2];
	  if ((bytes_read = (*ctx->read)(ctx->r_ctx, buf + 3, maxbuf - 3,
					 (size_t)buf[2])) != bytes_expected)
	       goto io_error;

	  if (cmd->t.tool.subcmd_len == 1)
	       cmd->t.tool.subcmd_value = (uint16_t)buf[3];
	  else if (cmd->t.tool.subcmd_len > 1)
	       memcpy((void *)&cmd->t.tool.subcmd_value, buf + 3, sizeof(uint16_t));
	  else
	       cmd->t.tool.subcmd_value = 0;

	  maxbuf -= 3 + bytes_read;
	  buf    += 3 + bytes_read;

	  cmd->t.tool.subcmd_desc = tool_command_table[cmd->t.tool.subcmd_id].cmd_desc;
	  if (cmd->t.tool.subcmd_desc == NULL)
	       cmd->t.tool.subcmd_desc = "unknown tool subcommand";
	  break;

     case HOST_CMD_SET_POSITION_EXT :
	  // x4, y4, z4, a4, b4 = 20 bytes
	  GET_INT32(set_position_ext.x);
	  GET_INT32(set_position_ext.y);
	  GET_INT32(set_position_ext.z);
	  GET_INT32(set_position_ext.a);
	  GET_INT32(set_position_ext.b);
	  break;

     case HOST_CMD_QUEUE_POINT_EXT :
	  // x4, y4, z4, a4, b4, dda4 = 24 bytes
	  GET_INT32(queue_point_ext.x);
	  GET_INT32(queue_point_ext.y);
	  GET_INT32(queue_point_ext.z);
	  GET_INT32(queue_point_ext.a);
	  GET_INT32(queue_point_ext.b);
	  GET_INT32(queue_point_ext.dda);
	  ZERO(queue_point_ext.dummy_rel, uint8_t);
	  ZERO(queue_point_ext.dummy_distance, float);
	  ZERO(queue_point_ext.dummy_feedrate_mult_64, uint16_t);
	  break;

     case HOST_CMD_QUEUE_POINT_NEW :
	  // x4, y4, z4, a4, b4, us4, relative = 25 bytes
	  GET_INT32(queue_point_new.x);
	  GET_INT32(queue_point_new.y);
	  GET_INT32(queue_point_new.z);
	  GET_INT32(queue_point_new.a);
	  GET_INT32(queue_point_new.b);
	  GET_INT32(queue_point_new.us);
	  GET_UINT8(queue_point_new.rel);
	  ZERO(queue_point_ext.dummy_distance, float);
	  ZERO(queue_point_ext.dummy_feedrate_mult_64, uint16_t);
	  break;

     case HOST_CMD_QUEUE_POINT_NEW_EXT :
	  // x4, y4, z4, a4, b4, dda_rate4, relative, distance 4, feedrate_mult64 2 = 31 bytes
	  GET_INT32(queue_point_new_ext.x);
	  GET_INT32(queue_point_new_ext.y);
	  GET_INT32(queue_point_new_ext.z);
	  GET_INT32(queue_point_new_ext.a);
	  GET_INT32(queue_point_new_ext.b);
	  GET_INT32(queue_point_new_ext.dda_rate);
	  GET_UINT8(queue_point_new_ext.rel);
	  GET_FLOAT32(queue_point_new_ext.distance);
	  GET_INT16(queue_point_new_ext.feedrate_mult_64);
	  break;

     case HOST_CMD_SET_POT_VALUE :
	  GET_UINT8(digi_pot.axis);
	  GET_UINT8(digi_pot.value);
	  break;

     case HOST_CMD_SET_RGB_LED :
	  GET_UINT8(rgb_led.red);
	  GET_UINT8(rgb_led.green);
	  GET_UINT8(rgb_led.blue);
	  GET_UINT8(rgb_led.blink_rate);
	  GET_UINT8(rgb_led.effect);
	  break;

     case HOST_CMD_SET_BEEP :
	  GET_UINT16(beep.frequency);
	  GET_UINT16(beep.duration);
	  GET_UINT8(beep.effect);
	  break;

     case HOST_CMD_PAUSE_FOR_BUTTON :
	  GET_UINT8(button_pause.mask);
	  GET_UINT16(button_pause.timeout);
	  GET_UINT8(button_pause.timeout_behavior);
	  break;

     case HOST_CMD_DISPLAY_MESSAGE :
	  GET_UINT8(display_message.options);
	  GET_UINT8(display_message.x);
	  GET_UINT8(display_message.y);
	  GET_UINT8(display_message.timeout);
	  cmd->t.display_message.message_len = 0;
	  if (maxbuf < 1) goto trunc;
	  for (;;)
	  {
	       unsigned char uc;
	       if (1 != (bytes_read = (*ctx->read)(ctx->r_ctx, buf, maxbuf, 1)))
		    goto io_error;
	       uc = buf[0];
	       ++buf;
	       --maxbuf;
	       if (uc == '\0')
		    break;
	       if (cmd->t.display_message.message_len < (sizeof(cmd->t.display_message.message) - 1))
		    cmd->t.display_message.message[cmd->t.display_message.message_len++] = uc;
	       if (maxbuf < 1) goto trunc;
	  }
	  cmd->t.display_message.message[cmd->t.display_message.message_len] = '\0';
	  break;

     case HOST_CMD_SET_BUILD_PERCENT :
	  GET_UINT8(build_percentage.percentage);
	  GET_UINT8(build_percentage.reserved);
	  break;

     case HOST_CMD_QUEUE_SONG :
	  GET_UINT8(queue_song.song_id);
	  break;

     case HOST_CMD_RESET_TO_FACTORY :
	  GET_UINT8(factory_reset.options);
	  break;

     case HOST_CMD_BUILD_START_NOTIFICATION :
	  GET_INT32(build_start.steps);
	  cmd->t.build_start.message_len = 0;
	  if (maxbuf < 1) goto trunc;
	  for (;;)
	  {
	       unsigned char uc;
	       if (1 != (bytes_read = (*ctx->read)(ctx->r_ctx, buf, maxbuf, 1)))
		    goto io_error;
	       uc = buf[0];
	       ++buf;
	       --maxbuf;
	       if (uc == '\0')
		    break;
	       if (cmd->t.build_start.message_len < (sizeof(cmd->t.build_start.message) - 1))
		    cmd->t.build_start.message[cmd->t.build_start.message_len++] = uc;
	  }
	  cmd->t.build_start.message[cmd->t.build_start.message_len] = '\0';
	  break;

     case HOST_CMD_BUILD_END_NOTIFICATION :
	  GET_UINT8(build_end.flags);
	  break;

     case HOST_CMD_CHANGE_TOOL :
	  GET_UINT8(change_tool.index);
          break;

     case HOST_CMD_ENABLE_AXES :
	  GET_UINT8(enable_axes.axes);
          break;

     case HOST_CMD_SET_ACCELERATION_TOGGLE:
	  GET_UINT8(set_segment_acceleration.s);
	  break;

     case HOST_CMD_STREAM_VERSION:
	  GET_UINT8(x3g_version.version_high);
	  GET_UINT8(x3g_version.version_low);
	  GET_UINT8(x3g_version.reserved1);
	  GET_UINT32(x3g_version.reserved2);
	  GET_UINT16(x3g_version.bot_type);
	  GET_UINT16(x3g_version.reserved3);
	  GET_UINT32(x3g_version.reserved4);
	  GET_UINT32(x3g_version.reserved5);
	  GET_UINT8(x3g_version.reserved6);
	  break;
     }

#undef ZERO
#undef GET_UINT8
#undef GET_INT32

     iret = 0;
     goto done;

io_error:
     fprintf(stderr,
	     "s3g_command_get(%d): Error while reading from the s3g file; "
	     "%s (%d)\n",
	     __LINE__, strerror(errno), errno);
     iret = -1;
     goto done;

trunc:
     fprintf(stderr,
	     "s3g_command_get(%d): Caller supplied read buffer is too small",
	     __LINE__);
     iret = -1;

done:
     cmd->cmd_raw_len = (size_t)(buf - buf0);
     if (buflen)
	  *buflen = cmd->cmd_raw_len;

     return(iret);
}

static void writef(s3g_context_t *ctx, const char *fmt, ...)
{
	va_list ap;
	char buf[4096];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (ctx && ctx->write)
		(*ctx->write)(ctx->w_ctx, (unsigned char *)buf, strlen(buf));
	else
		puts(buf);
}

int s3g_command_read(s3g_context_t *ctx, s3g_command_t *cmd)
{
     if (cmd)
	  return(s3g_command_read_ext(ctx, cmd, cmd->cmd_raw, sizeof(cmd->cmd_raw), NULL));
     else
     {
	  unsigned char buf[128];
	  return(s3g_command_read_ext(ctx, cmd, buf, sizeof(buf), NULL));
     }
}

static const char *bot_type(uint16_t btype, char *buf, size_t maxbuf)
{
    char tmpbuf[32];

    if (!buf || !maxbuf)
    {
	buf = tmpbuf;
	maxbuf = sizeof(tmpbuf);
    }

    if (btype == 0xD314)
	snprintf(buf, maxbuf, "Replicator 1");
    else if (btype == 0xB015)
	snprintf(buf, maxbuf, "Replicator 2");
    else
	snprintf(buf, maxbuf, "Unknown");

    return(buf);
}

static const char *axes_mask(uint8_t flags, char *buf, size_t maxbuf,
			     int isEnable)
{

#define CAT(x) \
  { if (buf[0]) strncat(buf, ", ", maxbuf); \
    strncat(buf, (x), maxbuf); }

	if (!buf || !maxbuf)
		return(NULL);

	buf[0] = '\0';

	if (isEnable) strncat(buf, (flags & 0x80) ? "+" : "-", maxbuf);

	if (flags != (uint8_t)0)
	{
	     if (flags & 0x01) CAT("X");
	     if (flags & 0x02) CAT("Y");
	     if (flags & 0x04) CAT("Z");
	     if (flags & 0x08) CAT("A");
	     if (flags & 0x10) CAT("B");
	}
	else
	     strncat(buf, "<NO AXES>", maxbuf);

	return(buf);

#undef CAT
}

static const char *axes_names(uint8_t axis, char *buf, size_t maxbuf)
{
     static const char *names[] = {"X", "Y", "Z", "A", "B"};
    
     if (!buf || !maxbuf)
	  return(NULL);

     if (axis <= (sizeof(names)/sizeof(const char *)))
	  strncpy(buf, names[axis], maxbuf);
     else
	  snprintf(buf, maxbuf, "unknown %hhu", axis);

     return(buf);
}

int s3g_add_writer(s3g_context_t *ctx, s3g_write_proc_t *wproc, void *wctx)
{
	if (!ctx)
		return(-1);

	ctx->write = wproc;
	ctx->w_ctx = wctx;

	return(0);
}

void s3g_command_display(s3g_context_t *ctx, s3g_command_t *cmd)
{
     char buf[64];
     const char *fmt;

#define F(v) (cmd->t.v)

     switch(cmd->cmd_id)
     {
     default :
	  writef(ctx, "%s", cmd->cmd_desc);
	  break;

     case HOST_CMD_DELAY :
	  writef(ctx, "Pause for %u ms", F(delay.millis));
	  break;

     case HOST_CMD_FIND_AXES_MINIMUM :
	  writef(ctx, "Home minimum on %s, feedrate %u us/step, timeout %hu s",
		 axes_mask(F(find_axes_minmax.flags), buf, sizeof(buf), 0),
		 F(find_axes_minmax.feedrate),
		 F(find_axes_minmax.timeout));
	  break;

     case HOST_CMD_FIND_AXES_MAXIMUM :
	  writef(ctx, "Home maximum on %s, feedrate %u us/step, timeout %hu s",
		 axes_mask(F(find_axes_minmax.flags), buf, sizeof(buf), 0),
		 F(find_axes_minmax.feedrate),
		 F(find_axes_minmax.timeout));
	  break;

     case HOST_CMD_WAIT_FOR_TOOL :
	  writef(ctx, "Wait for tool %hhu, poll delay %hu ms, timeout %hu s",
		 F(wait_for_tool.index),
		 F(wait_for_tool.ping_delay),
		 F(wait_for_tool.timeout));
	  break;

     case HOST_CMD_WAIT_FOR_PLATFORM :
	  writef(ctx,
		 "Wait for platform %hhu, poll delay %hu ms, timeout %hu s",
		 F(wait_for_tool.index),
		 F(wait_for_tool.ping_delay),
		 F(wait_for_tool.timeout));
	  break;

     case HOST_CMD_STORE_HOME_POSITION :
	  writef(ctx, "Store home position for %s",
		 axes_mask(F(store_home_position.axes), buf, sizeof(buf), 0));
	  break;

     case HOST_CMD_RECALL_HOME_POSITION :
	  writef(ctx, "Recall home position for %s",
		 axes_mask(F(recall_home_position.axes), buf, sizeof(buf), 0));
	  break;

     case HOST_CMD_TOOL_COMMAND :
	  switch (cmd->t.tool.subcmd_id)
	  {
	  case SLAVE_CMD_SET_TEMP :
	       fmt = "Set tool %hhu target temperature to %hu";
	       break;

	  case SLAVE_CMD_SET_PLATFORM_TEMP :
	       fmt = "Set platform %hhu temperature to %hu C";
	       break;

	  case SLAVE_CMD_SET_MOTOR_1_PWM :
	       fmt = "Tool %hhu, set motor 1 PWM to %hu";
	       break;

	  case SLAVE_CMD_TOGGLE_MOTOR_1 :
	       fmt = "Tool %hhu, toggle motor 1 to %hu";
	       break;

	  case SLAVE_CMD_TOGGLE_VALVE :
	       if (cmd->t.tool.subcmd_value == 0)
		    fmt = "Close valve (%hhu, %hu)";
	       else
		    fmt = "Open valve (%hhu, %hu)";
	       break;

	  default :
	       fmt = NULL;
	       break;
	  }
	  if (fmt)
	       writef(ctx, fmt, cmd->t.tool.index, cmd->t.tool.subcmd_value);
	  else
	       writef(ctx, "Tool command %hhu for tool %hhu, value %hu",
		      cmd->t.tool.subcmd_id,
		      cmd->t.tool.index,
		      cmd->t.tool.subcmd_value);
	  break;

     case HOST_CMD_SET_POSITION_EXT :
	  writef(ctx, "Set position to (%d, %d, %d, %d, %d)",
		 F(set_position_ext.x),
		 F(set_position_ext.y),
		 F(set_position_ext.z),
		 F(set_position_ext.a),
		 F(set_position_ext.b));
	  break;

     case HOST_CMD_QUEUE_POINT_EXT :
	  writef(ctx, "Move to (%d, %d, %d, %d, %d), with DDA %d",
		 F(queue_point_ext.x),
		 F(queue_point_ext.y),
		 F(queue_point_ext.z),
		 F(queue_point_ext.a),
		 F(queue_point_ext.b),
		 F(queue_point_ext.dda));
	  break;

     case HOST_CMD_QUEUE_POINT_NEW :
	  writef(ctx, "Move to (%d, %d, %d, %d, %d), in %d us, %s relative ",
		 F(queue_point_new.x),
		 F(queue_point_new.y),
		 F(queue_point_new.z),
		 F(queue_point_new.a),
		 F(queue_point_new.b),
		 F(queue_point_new.us),
		 axes_mask(F(queue_point_new.rel), buf, sizeof(buf), 0));
	  break;

     case HOST_CMD_QUEUE_POINT_NEW_EXT :
	  writef(ctx, "Move to (%d, %d, %d, %d, %d), DDA rate %d, %s relative, "
		 "distance %f mm, feedrate*64 %d steps/s",
		 F(queue_point_new_ext.x),
		 F(queue_point_new_ext.y),
		 F(queue_point_new_ext.z),
		 F(queue_point_new_ext.a),
		 F(queue_point_new_ext.b),
		 F(queue_point_new_ext.dda_rate),
		 axes_mask(F(queue_point_new_ext.rel), buf, sizeof(buf), 0),
		 F(queue_point_new_ext.distance),
		 F(queue_point_new_ext.feedrate_mult_64));
	  break;

     case HOST_CMD_SET_POT_VALUE :
	  writef(ctx, "Set %s axis potentiometer to %hhu",
		 axes_names(F(digi_pot.axis), buf, sizeof(buf)),
		 F(digi_pot.value));
	  break;

     case HOST_CMD_SET_RGB_LED :
	  writef(ctx, "Set RGB LED strip (0x%02hhx, 0x%02hhx, 0x%02hhx), "
		 "blink rate %hhu, effect %hhu",
		 F(rgb_led.red),
		 F(rgb_led.green),
		 F(rgb_led.blue),
		 F(rgb_led.blink_rate),
		 F(rgb_led.effect));
	  break;

     case HOST_CMD_SET_BEEP :
	  writef(ctx, "Set buzzer frequency %hu, duration %hu ms, effect %hhu",
		 F(beep.frequency),
		 F(beep.duration),
		 F(beep.effect));
	  break;

     case HOST_CMD_PAUSE_FOR_BUTTON :
	  writef(ctx, "Pause for button 0x%02hhx, timeout %hu s, "
		 "timeout behavior %hhu",
		 F(button_pause.mask),
		 F(button_pause.timeout),
		 F(button_pause.timeout_behavior));
	  break;

     case HOST_CMD_DISPLAY_MESSAGE :
	  writef(ctx, "Display at (%hhu,%hhu) the message \"%.*s\", "
		 "options 0x%02hhx, timeout %hhu s",
		 F(display_message.x),
		 F(display_message.y),
		 F(display_message.message_len),
		 F(display_message.message),
		 F(display_message.options),
		 F(display_message.timeout));
	  break;

     case HOST_CMD_SET_BUILD_PERCENT :
	  writef(ctx, "Set build percentage to %hhu%%, reserved %hhu",
		 F(build_percentage.percentage),
		 F(build_percentage.reserved));
	  break;

     case HOST_CMD_QUEUE_SONG :
	  writef(ctx, "Queue song %hhu", F(queue_song.song_id));
	  break;

     case HOST_CMD_RESET_TO_FACTORY :
	  writef(ctx, "Restore factory settings, options 0x%02hhx",
		 F(factory_reset.options));
	  break;

     case HOST_CMD_BUILD_START_NOTIFICATION :
	  writef(ctx, "Start build \"%.*s\", steps %d",
		 F(build_start.message_len),
		 F(build_start.message),
		 F(build_start.steps));
	  break;

     case HOST_CMD_BUILD_END_NOTIFICATION :
	  writef(ctx, "Build end, flags 0x%02hhx", F(build_end.flags));
	  break;

     case HOST_CMD_CHANGE_TOOL :
	  writef(ctx, "Switch to tool %hhu", F(change_tool.index));
          break;

     case HOST_CMD_ENABLE_AXES :
	  writef(ctx, "%s axes %s",
		 (F(enable_axes.axes) & 0x80) ? "Enable" : "Disable",
		 axes_mask(F(enable_axes.axes), buf, sizeof(buf), 0));
          break;

     case HOST_CMD_SET_ACCELERATION_TOGGLE:
	  writef(ctx, "Set segment acceleration %s", (F(set_segment_acceleration.s)) ? "on" : "off");
	  break;

     case HOST_CMD_STREAM_VERSION:
	  writef(ctx, "x3g stream version %hhu.%hhu, bot type %s (0x%04hx)",
		 F(x3g_version.version_high), F(x3g_version.version_low),
		 bot_type(F(x3g_version.bot_type), buf, sizeof(buf)),
		 F(x3g_version.bot_type));
	  break;
     }
}

int s3g_command_write(s3g_context_t *ctx, s3g_command_t *cmd)
{
     if (!ctx || !cmd || !ctx->write)
     {
	  errno = EINVAL;
	  return(-1);
     }

     if (cmd->cmd_raw_len == 0)
	  return(0);

     if ((ssize_t)cmd->cmd_raw_len == (*ctx->write)(ctx->w_ctx, cmd->cmd_raw, cmd->cmd_raw_len))
	  return(0);

     return(-1);
}
