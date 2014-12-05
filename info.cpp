#include <gst/gst.h>
#include <unistd.h>
#include <string>
#include <iostream>

static gboolean
bus_call (GstBus * bus, GstMessage * msg, gpointer data)
{
//  g_print ("bus_call here!\n");
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_EOS:{
      g_print ("End-of-stream\n");
      g_main_loop_quit (loop);
      break;
    }
    case GST_MESSAGE_ERROR:{
      gchar *debug;
      GError *err;

      gst_message_parse_error (msg, &err, &debug);
      g_printerr ("Debugging info: %s\n", (debug) ? debug : "none");
      g_free (debug);

      g_print ("Error: %s\n", err->message);
      g_error_free (err);

      g_main_loop_quit (loop);

      break;
    }
    default:
      break;
  }
  return TRUE;
}

static void
read_video_props (GstCaps *caps)
{
  gint width, height, num, denom;
  const GstStructure *str;
  const char *format;
  const char *type;

  g_return_if_fail (gst_caps_is_fixed (caps));

  str = gst_caps_get_structure (caps, 0);
  type = gst_structure_get_name (str);
  format = gst_structure_get_string (str, "format");
  if (!gst_structure_get_int (str, "width", &width) ||
      !gst_structure_get_int (str, "height", &height) ||
      !gst_structure_get_fraction (str, "framerate", &num, &denom)) {
    g_print ("No width/height available\n");
    return;
   }

  g_print ("The video size of this set of capabilities is %d x %d and the frame rate is %d/%d\nformat:%s type:%s\n",width, height,num,denom,format,type);
  g_print("caps: %s\n", gst_caps_to_string(caps));
}

gint
main (gint argc, gchar * argv[])
{
  GstElement *playbin, *fakevideosink, *fakeaudiosink;
  GMainLoop *loop;
  GstBus *bus;
  guint bus_watch_id;
  gchar *uri;
  GstState state = GST_STATE_NULL;
  GstPad *pad;
  GstCaps *caps;
  GstBuffer *buffer;
  std::string filename = "/Users/Loli/video/duoyan.mp4";

  gst_init (NULL, NULL);

  playbin = gst_element_factory_make ("playbin", NULL);
  if (!playbin) {
    g_print ("'playbin' gstreamer plugin missing\n");
    return 1;
  }

  /* take the commandline argument and ensure that it is a uri */
  if(argc >=2 )
    filename = argv[1];
  if (gst_uri_is_valid (filename.c_str()))
    uri = g_strdup (filename.c_str());
  else
    uri = gst_filename_to_uri (filename.c_str(), NULL);

  fakevideosink = gst_element_factory_make ("fakesink", NULL);
  fakeaudiosink = gst_element_factory_make ("fakesink", NULL);

  g_object_set (playbin, "uri", uri, NULL);
  g_free (uri);

  g_object_set (playbin, "video-sink", fakevideosink, NULL);
  g_object_set (playbin, "audio-sink", fakeaudiosink, NULL);

  gst_element_set_state (playbin, GST_STATE_PAUSED);
  gst_element_get_state (playbin, &state, NULL, GST_SECOND * 1);
  //fail_unless (state == GST_STATE_PAUSED);
  g_signal_emit_by_name (playbin, "get-video-pad", 0, &pad, NULL);
  GstSample *sample;
//  g_object_set (playbin, "sample", &sample, NULL);
  g_signal_emit_by_name (playbin, "convert-sample",NULL, &sample);
  buffer = gst_sample_get_buffer (sample);
  std::cout << gst_buffer_get_size (buffer) << std::endl;

  caps =  gst_pad_get_current_caps (pad);
  read_video_props(caps);

  /* create and event loop and feed gstreamer bus mesages to it */
  loop = g_main_loop_new (NULL, FALSE);

  bus = gst_element_get_bus (playbin);
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  g_object_unref (bus);

  /* start play back and listed to events */
  gst_element_set_state (playbin, GST_STATE_PLAYING);
  g_main_loop_run (loop);

  /* cleanup */
  gst_element_set_state (playbin, GST_STATE_NULL);
  g_object_unref (playbin);
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);
//  sleep(300);

  return 0;
}
