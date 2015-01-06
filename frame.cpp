#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <time.h>

int number=0;
int frameTotal = 16050;

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

static GstFlowReturn 
new_sample (GstElement *sink, GMainLoop *loop) 
{
  GstSample *sample = NULL;
  /* Retrieve the buffer */
  //g_print("new_sample here!\n");
  GstCaps *caps;
  GstBuffer *buf;
  g_signal_emit_by_name (sink, "pull-sample", &sample);
  caps = gst_sample_get_caps (sample);
//  if (*number == 0)
//  read_video_props(caps);
  buf = gst_sample_get_buffer (sample);
  if (sample) 
  {
//    (*number) ++;
//   g_print ("framenumber : %d\n", *number);
    number ++;
    g_print ("framenumber : %d\n", number);
    gst_sample_unref (sample);
    if (number == frameTotal)
      g_main_loop_quit(loop);
  }
  return GST_FLOW_OK;
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
  GstSample *sample;
  const GstStructure * info;
  GstSegment *segment;
  GstMapInfo map;
  std::string filename = "/Users/Lijing/Movies/duoyan.mp4";

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

  fakevideosink = gst_element_factory_make ("appsink", NULL);
  fakeaudiosink = gst_element_factory_make ("appsink", NULL);

  g_object_set (playbin, "uri", uri, NULL);
  g_free (uri);

  g_object_set (playbin, "video-sink", fakevideosink, NULL);
  g_object_set (playbin, "audio-sink", fakeaudiosink, NULL);

  /* read_info part
  gst_element_set_state (playbin, GST_STATE_PAUSED);
  gst_element_get_state (playbin, &state, NULL, GST_SECOND * 1);
  g_signal_emit_by_name (playbin, "get-video-pad", 0, &pad, NULL);
  caps =  gst_pad_get_current_caps (pad);
  read_video_props(caps);
  */

//  g_object_set (playbin, "sample", &sample, NULL); problem

  /* create and event loop and feed gstreamer bus mesages to it 
  */
  loop = g_main_loop_new (NULL, FALSE);
  bus = gst_element_get_bus (playbin);
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  g_object_unref (bus);

  g_object_set (G_OBJECT (fakevideosink), "sync", FALSE, NULL); 
//  g_object_set (fakevideosink, "emit-signals", TRUE, NULL);
//  g_signal_connect (fakevideosink, "new-sample", G_CALLBACK (new_sample), loop);

  time_t time_start = time(0);
  /* start play back and listed to events */
  gst_element_set_state (playbin, GST_STATE_PLAYING);

  int framenumber = 0;

  do {
    g_signal_emit_by_name (fakevideosink, "pull-sample", &sample);
    g_print("After calling pull-sample  ");
    if (sample == NULL){
      g_print("Meet the EOS!\n");
      g_main_loop_quit(loop);
      break;
      }
    framenumber ++;
    caps = gst_sample_get_caps(sample);
    buffer = gst_sample_get_buffer (sample);
    info = gst_sample_get_info(sample);
    segment = gst_sample_get_segment(sample);
    gst_buffer_map (buffer, &map, GST_MAP_READ);
    std::cout << "Frame number: "<<framenumber <<std::endl;

    if(framenumber < 1000)
    {
      std::cout << "Gstbuffer size: " << gst_buffer_get_size (buffer) << std::endl;
      std::cout << "Map size: " << map.size << std::endl;
      std::cout << "Segment Info: position " << segment->position <<" duration " << segment->duration << std::endl;
      read_video_props(caps);
      if (info !=NULL)
        std::cout << "Sample INFO " << gst_structure_to_string (info) <<std::endl;
    }
//    g_print ("got sample: %p", sample);

      if (info !=NULL)
        std::cout << "Sample INFO " << gst_structure_to_string (info) <<std::endl;
    if (sample)
      gst_sample_unref (sample);
    }while (framenumber <= frameTotal);
  //  }while (sample != NULL);

    g_print("Should be here\n");
//  std::cout << seconds << " seconds have passed" << std::endl;

//  do {
//    g_signal_emit_by_name (fakeaudiosink, "pull-sample", &sample);
//    buffer = gst_sample_get_buffer (sample);
//    gst_buffer_map (buffer, &map, GST_MAP_READ);
//    std::cout << "Map size: " << map.size << std::endl;
//
//    if (sample)
//      gst_sample_unref (sample);
//    }while (sample != NULL);

//  g_main_loop_run (loop);
  time_t time_end = time(0);
  double seconds = difftime(time_end, time_start);
  std::cout << seconds << " seconds have passed" << std::endl;

  /* cleanup */
  gst_element_set_state (playbin, GST_STATE_NULL);
  g_object_unref (playbin);
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);
//  sleep(300);

  return 0;
}
