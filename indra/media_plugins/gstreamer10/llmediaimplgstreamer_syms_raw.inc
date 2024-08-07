#define G gstSymbolGrabber

LL_GRAB_SYM(G, true, gst_buffer_new, GstBuffer*, void)
LL_GRAB_SYM(G, true, gst_structure_set_value, void, GstStructure *, const gchar *, const GValue*)
LL_GRAB_SYM(G, true, gst_init_check, gboolean, int *argc, char **argv[], GError ** err)
LL_GRAB_SYM(G, true, gst_message_get_type, GType, void)
LL_GRAB_SYM(G, true, gst_message_type_get_name, const gchar*, GstMessageType type)
LL_GRAB_SYM(G, true, gst_message_parse_error, void, GstMessage *message, GError **gerror, gchar **debug)
LL_GRAB_SYM(G, true, gst_message_parse_warning, void, GstMessage *message, GError **gerror, gchar **debug)
LL_GRAB_SYM(G, true, gst_message_parse_state_changed, void, GstMessage *message, GstState *oldstate, GstState *newstate, GstState *pending)
LL_GRAB_SYM(G, true, gst_element_set_state, GstStateChangeReturn, GstElement *element, GstState state)
LL_GRAB_SYM(G, true, gst_object_unref, void, gpointer object)
LL_GRAB_SYM(G, true, gst_object_get_type, GType, void)
LL_GRAB_SYM(G, true, gst_pipeline_get_type, GType, void)
LL_GRAB_SYM(G, true, gst_pipeline_get_bus, GstBus*, GstPipeline *pipeline)
LL_GRAB_SYM(G, true, gst_bus_add_watch, guint, GstBus * bus, GstBusFunc func, gpointer user_data)
LL_GRAB_SYM(G, true, gst_element_factory_make, GstElement*, const gchar *factoryname, const gchar *name)
LL_GRAB_SYM(G, true, gst_element_get_type, GType, void)
LL_GRAB_SYM(G, true, gst_static_pad_template_get, GstPadTemplate*, GstStaticPadTemplate *pad_template)
LL_GRAB_SYM(G, true, gst_element_class_add_pad_template, void, GstElementClass *klass, GstPadTemplate *temp)
LL_GRAB_SYM(G, true, gst_caps_from_string, GstCaps *, const gchar *string)
LL_GRAB_SYM(G, true, gst_caps_get_structure, GstStructure *, const GstCaps *caps, guint index)
LL_GRAB_SYM(G, true, gst_element_register, gboolean, GstPlugin *plugin, const gchar *name, guint rank, GType type)
LL_GRAB_SYM(G, true, gst_structure_get_int, gboolean, const GstStructure *structure, const gchar *fieldname, gint *value)
LL_GRAB_SYM(G, true, gst_structure_get_value, const GValue *, const GstStructure *structure, const gchar *fieldname)
LL_GRAB_SYM(G, true, gst_value_get_fraction_numerator, gint, const GValue *value)
LL_GRAB_SYM(G, true, gst_value_get_fraction_denominator, gint, const GValue *value)
LL_GRAB_SYM(G, true, gst_structure_get_name, const gchar *, const GstStructure *structure)
LL_GRAB_SYM(G, true, gst_element_seek, bool, GstElement *, gdouble, GstFormat, GstSeekFlags, GstSeekType, gint64, GstSeekType, gint64)

LL_GRAB_SYM(G, false, gst_registry_fork_set_enabled, void, gboolean enabled)
LL_GRAB_SYM(G, false, gst_segtrap_set_enabled, void, gboolean enabled)
LL_GRAB_SYM(G, false, gst_message_parse_buffering, void, GstMessage *message, gint *percent)
LL_GRAB_SYM(G, false, gst_message_parse_info, void, GstMessage *message, GError **gerror, gchar **debug)
LL_GRAB_SYM(G, false, gst_element_query_position, gboolean, GstElement *element, GstFormat *format, gint64 *cur)
LL_GRAB_SYM(G, false, gst_version, void, guint *major, guint *minor, guint *micro, guint *nano)

LL_GRAB_SYM(G, true, gst_message_parse_tag, void, GstMessage *, GstTagList **)
LL_GRAB_SYM(G, true, gst_tag_list_foreach, void, const GstTagList *, GstTagForeachFunc, gpointer)
LL_GRAB_SYM(G, true, gst_tag_list_get_tag_size, guint, const GstTagList *, const gchar *)
LL_GRAB_SYM(G, true, gst_tag_list_get_value_index, const GValue *, const GstTagList *, const gchar *, guint)

LL_GRAB_SYM(G, true, gst_caps_new_simple, GstCaps*, const char *, const char*, ... )

LL_GRAB_SYM(G, true, gst_sample_get_caps, GstCaps*, GstSample* )
LL_GRAB_SYM(G, true, gst_sample_get_buffer, GstBuffer*, GstSample* )
LL_GRAB_SYM(G, true, gst_buffer_map, gboolean, GstBuffer*, GstMapInfo*, GstMapFlags )
LL_GRAB_SYM(G, true, gst_buffer_unmap, void, GstBuffer*, GstMapInfo* )

LL_GRAB_SYM(G, true, gst_app_sink_set_caps, void, GstAppSink*, GstCaps const* )
LL_GRAB_SYM(G, true, gst_app_sink_pull_sample, GstSample*, GstAppSink* )

LL_GRAB_SYM(G, true, g_free, void, gpointer )
LL_GRAB_SYM(G, true, g_error_free, void, GError* )

LL_GRAB_SYM(G, true, g_main_context_pending, gboolean, GMainContext* )
LL_GRAB_SYM(G, true, g_main_loop_get_context, GMainContext*, GMainLoop* )
LL_GRAB_SYM(G, true, g_main_context_iteration, gboolean, GMainContext*, gboolean )
LL_GRAB_SYM(G, true, g_main_loop_new, GMainLoop*, GMainContext*, gboolean )
LL_GRAB_SYM(G, true, g_main_loop_quit, void, GMainLoop* )
LL_GRAB_SYM(G, true, gst_mini_object_unref, void, GstMiniObject* )
LL_GRAB_SYM(G, true, g_object_set, void, gpointer, gchar const*, ... )
LL_GRAB_SYM(G, true, g_source_remove, gboolean, guint )
LL_GRAB_SYM(G, true, g_value_get_string, gchar const*, GValue const* )

LL_GRAB_SYM(G, true, gst_debug_set_active, void, gboolean )
LL_GRAB_SYM(G, true, gst_debug_add_log_function, void, GstLogFunction, gpointer, GDestroyNotify )
LL_GRAB_SYM(G, true, gst_debug_set_default_threshold, void, GstDebugLevel )
LL_GRAB_SYM(G, true, gst_debug_message_get , gchar const*, GstDebugMessage * )

#undef G
