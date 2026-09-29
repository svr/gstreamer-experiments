#include <gst/gst.h>

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    GstElement *pipeline = gst_pipeline_new("pipeline");
    GstElement* videotestsrc = gst_element_factory_make( "videotestsrc", "source");
    GstElement* videoconvert = gst_element_factory_make( "videoconvert", "convert");
    GstElement* autovideosink = gst_element_factory_make( "autovideosink", "sink");

    gst_bin_add_many(GST_BIN(pipeline), videotestsrc, videoconvert, autovideosink, nullptr);
    gst_element_link_many(videotestsrc, videoconvert, autovideosink, nullptr);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    GMainLoop* loop = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;
}