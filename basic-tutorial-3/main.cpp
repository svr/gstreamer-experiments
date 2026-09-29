#include <gst/gst.h>
#include <atomic>

struct CustomData {
  GstElement* pipeline;
  GstElement* source;
  GstElement* audioconvert;
  GstElement* videoconvert;
  GstElement* resample;
  GstElement* audiosink;
  GstElement* videosink;
};

static void pad_added_handler(GstElement* src, GstPad* pad, CustomData* data);

int main(int argc, char* argv[]) {
  gst_init(&argc, &argv);

  std::atomic<bool> terminate = false;
  CustomData data;

  data.source = gst_element_factory_make("uridecodebin", "source");
  data.audioconvert = gst_element_factory_make("audioconvert", "audioconvert");
  data.videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
  data.resample = gst_element_factory_make("audioresample", "resample");
  data.audiosink = gst_element_factory_make("autoaudiosink", "audiosink");
  data.videosink = gst_element_factory_make("autovideosink", "videosink");
  data.pipeline = gst_pipeline_new("pipeline");

  if(!data.pipeline || !data.source || !data.audioconvert || !data.videoconvert || !data.resample || !data.audiosink || !data.videosink) {
    g_printerr("Not all elements could be created.\n");
    return -1;
  }

  gst_bin_add_many(GST_BIN(data.pipeline), data.source, data.audioconvert, data.videoconvert, data.resample, data.audiosink, data.videosink, NULL);
  if(!gst_element_link_many(data.audioconvert, data.resample, data.audiosink, NULL)) {
    g_printerr("Audio elements could not be linked.\n");
    gst_object_unref(data.pipeline);
    return -1;
  }

  if(!gst_element_link(data.videoconvert, data.videosink)) {
    g_printerr("Video elements could not be linked.\n");
    gst_object_unref(data.pipeline);
    return -1;
  }

  g_object_set(data.source, "uri", "https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm", nullptr);
  g_signal_connect(data.source, "pad-added", G_CALLBACK(pad_added_handler), &data);

  auto ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
  if(ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr("Unable to set the pipeline to the playing state.\n");
    gst_object_unref(data.pipeline);
    return -1;
  }

  auto* bus = gst_element_get_bus(data.pipeline);
  while(!terminate) {
    auto* msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, static_cast<GstMessageType>(GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    if(msg != nullptr) {
      GError *err;
      gchar *debug_info;

      switch(GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_ERROR:
          gst_message_parse_error(msg, &err, &debug_info);
          g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
          g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
          g_clear_error(&err);
          g_free(debug_info);
          terminate = true;
          break;
        case GST_MESSAGE_EOS:
          g_print("End-Of-Stream reached.\n");
          terminate = true;
          break;
        case GST_MESSAGE_STATE_CHANGED:
          if(GST_MESSAGE_SRC(msg) == GST_OBJECT(data.pipeline)) {
            GstState old_state, new_state, pending_state;
            gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
            g_print("Pipeline state changed from %s to %s:\n", gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
          }
          break;
        default:
          g_printerr("Unexpected message received.\n");
          break;
      }
      gst_message_unref(msg);
    }
  }

  gst_object_unref(bus);
  gst_element_set_state(data.pipeline, GST_STATE_NULL);
  gst_object_unref(data.pipeline);
  return 0;
}

/* This function will be called by the pad-added signal */
static void pad_added_handler(GstElement * src, GstPad * new_pad, CustomData * data) {
  GstPad* sink_pad;
  auto* new_pad_caps = gst_pad_get_current_caps(new_pad);
  auto* new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
  auto* new_pad_type = gst_structure_get_name(new_pad_struct);

  if(g_str_has_prefix(new_pad_type, "audio/x-raw")) {
    sink_pad = gst_element_get_static_pad(data->audioconvert, "sink");
  } else if(g_str_has_prefix(new_pad_type, "video/x-raw")) {
    sink_pad = gst_element_get_static_pad(data->videoconvert, "sink");
  } else {
    g_print("It has type '%s' which is not raw audio or raw video. Ignoring.\n", new_pad_type);
    gst_caps_unref(new_pad_caps);
    return;
  }

  GstPadLinkReturn ret;
  g_print("Received new pad '%s' from '%s':\n", GST_PAD_NAME(new_pad), GST_ELEMENT_NAME(src));

  if(gst_pad_is_linked(sink_pad)) {
    g_print("We are already linked. Ignoring.\n");
    gst_object_unref(sink_pad);
    gst_caps_unref(new_pad_caps);
    return;
  }

  ret = gst_pad_link(new_pad, sink_pad);
  if(GST_PAD_LINK_FAILED(ret)) {
    g_print("Type is '%s' but link failed.\n", new_pad_type);
  } else {
    g_print("Link succeeded(type '%s').\n", new_pad_type);
  }

  if(new_pad_caps != nullptr) {
    gst_caps_unref(new_pad_caps);
  }

  gst_object_unref(sink_pad);
}

