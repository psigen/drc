/**
 * Adapted from http://stackoverflow.com/a/6947758
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <ros/ros.h>

int SetSerialAttribs(int fd, int speed, int parity)
{
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0)
  {
    ROS_ERROR("error %d from tcgetattr: %s", errno, strerror(errno));
    return -1;
  }

  cfsetospeed (&tty, speed);
  cfsetispeed (&tty, speed);

  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
  // disable IGNBRK for mismatched speed tests; otherwise receive break
  // as \000 chars
  tty.c_iflag &= ~IGNBRK;         // ignore break signal
  tty.c_lflag = 0;                // no signaling chars, no echo,
  // no canonical processing
  tty.c_oflag = 0;                // no remapping, no delays
  tty.c_cc[VMIN]  = 0;            // read doesn't block
  tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

  tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
  // enable reading
  tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
  tty.c_cflag |= parity;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  if (tcsetattr (fd, TCSANOW, &tty) != 0)
  {
    ROS_ERROR("error %d from tcsetattr: %s", errno, strerror(errno));
    return -1;
  }
  return 0;
}

void SetBlocking(int fd, bool should_block)
{
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0)
  {
    ROS_ERROR("error %d from tggetattr: %s", errno, strerror(errno));
    return;
  }

  tty.c_cc[VMIN]  = should_block ? 1 : 0;
  tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

  if (tcsetattr (fd, TCSANOW, &tty) != 0)
    ROS_ERROR("error %d setting term attributes: %s", errno, strerror(errno));
}

/*
  Example usage:
  
  ...
  char *portname = "/dev/ttyUSB1"
  ...
  int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0)
  {
    ROS_ERROR("error %d opening %s: %s", errno, portname, strerror (errno));
    return;
  }

  set_interface_attribs (fd, B115200, 0);  // set speed to 115,200 bps, 8n1
  set_blocking (fd, 0);                // set no blocking

  write (fd, "hello!\n", 7);           // send 7 character greeting
  char buf [100];
  int n = read (fd, buf, sizeof buf);  // read up to 100 characters if ready to read
*/
