/* Generated by dbus-binding-tool; do not edit! */


#ifndef __dbus_glib_marshal_rc_shell_MARSHAL_H__
#define __dbus_glib_marshal_rc_shell_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

#ifdef G_ENABLE_DEBUG
#define g_marshal_value_peek_boolean(v)  g_value_get_boolean (v)
#define g_marshal_value_peek_char(v)     g_value_get_char (v)
#define g_marshal_value_peek_uchar(v)    g_value_get_uchar (v)
#define g_marshal_value_peek_int(v)      g_value_get_int (v)
#define g_marshal_value_peek_uint(v)     g_value_get_uint (v)
#define g_marshal_value_peek_long(v)     g_value_get_long (v)
#define g_marshal_value_peek_ulong(v)    g_value_get_ulong (v)
#define g_marshal_value_peek_int64(v)    g_value_get_int64 (v)
#define g_marshal_value_peek_uint64(v)   g_value_get_uint64 (v)
#define g_marshal_value_peek_enum(v)     g_value_get_enum (v)
#define g_marshal_value_peek_flags(v)    g_value_get_flags (v)
#define g_marshal_value_peek_float(v)    g_value_get_float (v)
#define g_marshal_value_peek_double(v)   g_value_get_double (v)
#define g_marshal_value_peek_string(v)   (char*) g_value_get_string (v)
#define g_marshal_value_peek_param(v)    g_value_get_param (v)
#define g_marshal_value_peek_boxed(v)    g_value_get_boxed (v)
#define g_marshal_value_peek_pointer(v)  g_value_get_pointer (v)
#define g_marshal_value_peek_object(v)   g_value_get_object (v)
#else /* !G_ENABLE_DEBUG */
/* WARNING: This code accesses GValues directly, which is UNSUPPORTED API.
 *          Do not access GValues directly in your code. Instead, use the
 *          g_value_get_*() functions
 */
#define g_marshal_value_peek_boolean(v)  (v)->data[0].v_int
#define g_marshal_value_peek_char(v)     (v)->data[0].v_int
#define g_marshal_value_peek_uchar(v)    (v)->data[0].v_uint
#define g_marshal_value_peek_int(v)      (v)->data[0].v_int
#define g_marshal_value_peek_uint(v)     (v)->data[0].v_uint
#define g_marshal_value_peek_long(v)     (v)->data[0].v_long
#define g_marshal_value_peek_ulong(v)    (v)->data[0].v_ulong
#define g_marshal_value_peek_int64(v)    (v)->data[0].v_int64
#define g_marshal_value_peek_uint64(v)   (v)->data[0].v_uint64
#define g_marshal_value_peek_enum(v)     (v)->data[0].v_long
#define g_marshal_value_peek_flags(v)    (v)->data[0].v_ulong
#define g_marshal_value_peek_float(v)    (v)->data[0].v_float
#define g_marshal_value_peek_double(v)   (v)->data[0].v_double
#define g_marshal_value_peek_string(v)   (v)->data[0].v_pointer
#define g_marshal_value_peek_param(v)    (v)->data[0].v_pointer
#define g_marshal_value_peek_boxed(v)    (v)->data[0].v_pointer
#define g_marshal_value_peek_pointer(v)  (v)->data[0].v_pointer
#define g_marshal_value_peek_object(v)   (v)->data[0].v_pointer
#endif /* !G_ENABLE_DEBUG */


/* BOOLEAN:INT64,POINTER */
extern void dbus_glib_marshal_rc_shell_BOOLEAN__INT64_POINTER (GClosure     *closure,
                                                               GValue       *return_value,
                                                               guint         n_param_values,
                                                               const GValue *param_values,
                                                               gpointer      invocation_hint,
                                                               gpointer      marshal_data);
void
dbus_glib_marshal_rc_shell_BOOLEAN__INT64_POINTER (GClosure     *closure,
                                                   GValue       *return_value G_GNUC_UNUSED,
                                                   guint         n_param_values,
                                                   const GValue *param_values,
                                                   gpointer      invocation_hint G_GNUC_UNUSED,
                                                   gpointer      marshal_data)
{
  typedef gboolean (*GMarshalFunc_BOOLEAN__INT64_POINTER) (gpointer     data1,
                                                           gint64       arg_1,
                                                           gpointer     arg_2,
                                                           gpointer     data2);
  register GMarshalFunc_BOOLEAN__INT64_POINTER callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;
  gboolean v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_BOOLEAN__INT64_POINTER) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_int64 (param_values + 1),
                       g_marshal_value_peek_pointer (param_values + 2),
                       data2);

  g_value_set_boolean (return_value, v_return);
}

/* BOOLEAN:POINTER,POINTER,POINTER,POINTER,POINTER,POINTER,POINTER,POINTER,POINTER,POINTER,POINTER */
extern void dbus_glib_marshal_rc_shell_BOOLEAN__POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER (GClosure     *closure,
                                                                                                                                         GValue       *return_value,
                                                                                                                                         guint         n_param_values,
                                                                                                                                         const GValue *param_values,
                                                                                                                                         gpointer      invocation_hint,
                                                                                                                                         gpointer      marshal_data);
void
dbus_glib_marshal_rc_shell_BOOLEAN__POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER (GClosure     *closure,
                                                                                                                             GValue       *return_value G_GNUC_UNUSED,
                                                                                                                             guint         n_param_values,
                                                                                                                             const GValue *param_values,
                                                                                                                             gpointer      invocation_hint G_GNUC_UNUSED,
                                                                                                                             gpointer      marshal_data)
{
  typedef gboolean (*GMarshalFunc_BOOLEAN__POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER) (gpointer     data1,
                                                                                                                                     gpointer     arg_1,
                                                                                                                                     gpointer     arg_2,
                                                                                                                                     gpointer     arg_3,
                                                                                                                                     gpointer     arg_4,
                                                                                                                                     gpointer     arg_5,
                                                                                                                                     gpointer     arg_6,
                                                                                                                                     gpointer     arg_7,
                                                                                                                                     gpointer     arg_8,
                                                                                                                                     gpointer     arg_9,
                                                                                                                                     gpointer     arg_10,
                                                                                                                                     gpointer     arg_11,
                                                                                                                                     gpointer     data2);
  register GMarshalFunc_BOOLEAN__POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;
  gboolean v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 12);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_BOOLEAN__POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_pointer (param_values + 1),
                       g_marshal_value_peek_pointer (param_values + 2),
                       g_marshal_value_peek_pointer (param_values + 3),
                       g_marshal_value_peek_pointer (param_values + 4),
                       g_marshal_value_peek_pointer (param_values + 5),
                       g_marshal_value_peek_pointer (param_values + 6),
                       g_marshal_value_peek_pointer (param_values + 7),
                       g_marshal_value_peek_pointer (param_values + 8),
                       g_marshal_value_peek_pointer (param_values + 9),
                       g_marshal_value_peek_pointer (param_values + 10),
                       g_marshal_value_peek_pointer (param_values + 11),
                       data2);

  g_value_set_boolean (return_value, v_return);
}

/* BOOLEAN:DOUBLE,POINTER */
extern void dbus_glib_marshal_rc_shell_BOOLEAN__DOUBLE_POINTER (GClosure     *closure,
                                                                GValue       *return_value,
                                                                guint         n_param_values,
                                                                const GValue *param_values,
                                                                gpointer      invocation_hint,
                                                                gpointer      marshal_data);
void
dbus_glib_marshal_rc_shell_BOOLEAN__DOUBLE_POINTER (GClosure     *closure,
                                                    GValue       *return_value G_GNUC_UNUSED,
                                                    guint         n_param_values,
                                                    const GValue *param_values,
                                                    gpointer      invocation_hint G_GNUC_UNUSED,
                                                    gpointer      marshal_data)
{
  typedef gboolean (*GMarshalFunc_BOOLEAN__DOUBLE_POINTER) (gpointer     data1,
                                                            gdouble      arg_1,
                                                            gpointer     arg_2,
                                                            gpointer     data2);
  register GMarshalFunc_BOOLEAN__DOUBLE_POINTER callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;
  gboolean v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_BOOLEAN__DOUBLE_POINTER) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_double (param_values + 1),
                       g_marshal_value_peek_pointer (param_values + 2),
                       data2);

  g_value_set_boolean (return_value, v_return);
}

/* BOOLEAN:POINTER */
extern void dbus_glib_marshal_rc_shell_BOOLEAN__POINTER (GClosure     *closure,
                                                         GValue       *return_value,
                                                         guint         n_param_values,
                                                         const GValue *param_values,
                                                         gpointer      invocation_hint,
                                                         gpointer      marshal_data);
void
dbus_glib_marshal_rc_shell_BOOLEAN__POINTER (GClosure     *closure,
                                             GValue       *return_value G_GNUC_UNUSED,
                                             guint         n_param_values,
                                             const GValue *param_values,
                                             gpointer      invocation_hint G_GNUC_UNUSED,
                                             gpointer      marshal_data)
{
  typedef gboolean (*GMarshalFunc_BOOLEAN__POINTER) (gpointer     data1,
                                                     gpointer     arg_1,
                                                     gpointer     data2);
  register GMarshalFunc_BOOLEAN__POINTER callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;
  gboolean v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 2);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_BOOLEAN__POINTER) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_pointer (param_values + 1),
                       data2);

  g_value_set_boolean (return_value, v_return);
}

/* BOOLEAN:INT,POINTER */
extern void dbus_glib_marshal_rc_shell_BOOLEAN__INT_POINTER (GClosure     *closure,
                                                             GValue       *return_value,
                                                             guint         n_param_values,
                                                             const GValue *param_values,
                                                             gpointer      invocation_hint,
                                                             gpointer      marshal_data);
void
dbus_glib_marshal_rc_shell_BOOLEAN__INT_POINTER (GClosure     *closure,
                                                 GValue       *return_value G_GNUC_UNUSED,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint G_GNUC_UNUSED,
                                                 gpointer      marshal_data)
{
  typedef gboolean (*GMarshalFunc_BOOLEAN__INT_POINTER) (gpointer     data1,
                                                         gint         arg_1,
                                                         gpointer     arg_2,
                                                         gpointer     data2);
  register GMarshalFunc_BOOLEAN__INT_POINTER callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;
  gboolean v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_BOOLEAN__INT_POINTER) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_int (param_values + 1),
                       g_marshal_value_peek_pointer (param_values + 2),
                       data2);

  g_value_set_boolean (return_value, v_return);
}

/* BOOLEAN:STRING,POINTER */
extern void dbus_glib_marshal_rc_shell_BOOLEAN__STRING_POINTER (GClosure     *closure,
                                                                GValue       *return_value,
                                                                guint         n_param_values,
                                                                const GValue *param_values,
                                                                gpointer      invocation_hint,
                                                                gpointer      marshal_data);
void
dbus_glib_marshal_rc_shell_BOOLEAN__STRING_POINTER (GClosure     *closure,
                                                    GValue       *return_value G_GNUC_UNUSED,
                                                    guint         n_param_values,
                                                    const GValue *param_values,
                                                    gpointer      invocation_hint G_GNUC_UNUSED,
                                                    gpointer      marshal_data)
{
  typedef gboolean (*GMarshalFunc_BOOLEAN__STRING_POINTER) (gpointer     data1,
                                                            gpointer     arg_1,
                                                            gpointer     arg_2,
                                                            gpointer     data2);
  register GMarshalFunc_BOOLEAN__STRING_POINTER callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;
  gboolean v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_BOOLEAN__STRING_POINTER) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_string (param_values + 1),
                       g_marshal_value_peek_pointer (param_values + 2),
                       data2);

  g_value_set_boolean (return_value, v_return);
}

/* BOOLEAN:POINTER,POINTER */
extern void dbus_glib_marshal_rc_shell_BOOLEAN__POINTER_POINTER (GClosure     *closure,
                                                                 GValue       *return_value,
                                                                 guint         n_param_values,
                                                                 const GValue *param_values,
                                                                 gpointer      invocation_hint,
                                                                 gpointer      marshal_data);
void
dbus_glib_marshal_rc_shell_BOOLEAN__POINTER_POINTER (GClosure     *closure,
                                                     GValue       *return_value G_GNUC_UNUSED,
                                                     guint         n_param_values,
                                                     const GValue *param_values,
                                                     gpointer      invocation_hint G_GNUC_UNUSED,
                                                     gpointer      marshal_data)
{
  typedef gboolean (*GMarshalFunc_BOOLEAN__POINTER_POINTER) (gpointer     data1,
                                                             gpointer     arg_1,
                                                             gpointer     arg_2,
                                                             gpointer     data2);
  register GMarshalFunc_BOOLEAN__POINTER_POINTER callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;
  gboolean v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_BOOLEAN__POINTER_POINTER) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_pointer (param_values + 1),
                       g_marshal_value_peek_pointer (param_values + 2),
                       data2);

  g_value_set_boolean (return_value, v_return);
}

G_END_DECLS

#endif /* __dbus_glib_marshal_rc_shell_MARSHAL_H__ */

#include <dbus/dbus-glib.h>
static const DBusGMethodInfo dbus_glib_rc_shell_methods[] = {
  { (GCallback) rc_shell_load_uri, dbus_glib_marshal_rc_shell_BOOLEAN__STRING_POINTER, 0 },
  { (GCallback) rc_shell_play, dbus_glib_marshal_rc_shell_BOOLEAN__POINTER, 48 },
  { (GCallback) rc_shell_pause, dbus_glib_marshal_rc_shell_BOOLEAN__POINTER, 85 },
  { (GCallback) rc_shell_stop, dbus_glib_marshal_rc_shell_BOOLEAN__POINTER, 123 },
  { (GCallback) rc_shell_prev, dbus_glib_marshal_rc_shell_BOOLEAN__POINTER, 160 },
  { (GCallback) rc_shell_next, dbus_glib_marshal_rc_shell_BOOLEAN__POINTER, 197 },
  { (GCallback) rc_shell_get_state, dbus_glib_marshal_rc_shell_BOOLEAN__POINTER_POINTER, 234 },
  { (GCallback) rc_shell_get_position, dbus_glib_marshal_rc_shell_BOOLEAN__POINTER_POINTER, 289 },
  { (GCallback) rc_shell_set_position, dbus_glib_marshal_rc_shell_BOOLEAN__INT64_POINTER, 345 },
  { (GCallback) rc_shell_get_duration, dbus_glib_marshal_rc_shell_BOOLEAN__POINTER_POINTER, 397 },
  { (GCallback) rc_shell_get_volume, dbus_glib_marshal_rc_shell_BOOLEAN__POINTER_POINTER, 454 },
  { (GCallback) rc_shell_set_volume, dbus_glib_marshal_rc_shell_BOOLEAN__DOUBLE_POINTER, 508 },
  { (GCallback) rc_shell_get_repeat_mode, dbus_glib_marshal_rc_shell_BOOLEAN__POINTER_POINTER, 558 },
  { (GCallback) rc_shell_set_repeat_mode, dbus_glib_marshal_rc_shell_BOOLEAN__INT_POINTER, 619 },
  { (GCallback) rc_shell_get_random_mode, dbus_glib_marshal_rc_shell_BOOLEAN__POINTER_POINTER, 676 },
  { (GCallback) rc_shell_set_random_mode, dbus_glib_marshal_rc_shell_BOOLEAN__INT_POINTER, 737 },
  { (GCallback) rc_shell_get_current_track, dbus_glib_marshal_rc_shell_BOOLEAN__POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER_POINTER, 794 },
};

const DBusGObjectInfo dbus_glib_rc_shell_object_info = {  1,
  dbus_glib_rc_shell_methods,
  17,
"org.supercat.RhythmCat.Shell\0LoadURI\0S\0uri\0I\0s\0\0org.supercat.RhythmCat.Shell\0Play\0S\0\0org.supercat.RhythmCat.Shell\0Pause\0S\0\0org.supercat.RhythmCat.Shell\0Stop\0S\0\0org.supercat.RhythmCat.Shell\0Prev\0S\0\0org.supercat.RhythmCat.Shell\0Next\0S\0\0org.supercat.RhythmCat.Shell\0GetState\0S\0state\0O\0F\0N\0i\0\0org.supercat.RhythmCat.Shell\0GetPosition\0S\0pos\0O\0F\0N\0x\0\0org.supercat.RhythmCat.Shell\0SetPosition\0S\0pos\0I\0x\0\0org.supercat.RhythmCat.Shell\0GetDuration\0S\0dura\0O\0F\0N\0x\0\0org.supercat.RhythmCat.Shell\0GetVolume\0S\0vol\0O\0F\0N\0d\0\0org.supercat.RhythmCat.Shell\0SetVolume\0S\0vol\0I\0d\0\0org.supercat.RhythmCat.Shell\0GetRepeatMode\0S\0repeat\0O\0F\0N\0i\0\0org.supercat.RhythmCat.Shell\0SetRepeatMode\0S\0repeat\0I\0i\0\0org.supercat.RhythmCat.Shell\0GetRandomMode\0S\0random\0O\0F\0N\0i\0\0org.supercat.RhythmCat.Shell\0SetRandomMode\0S\0random\0I\0i\0\0org.supercat.RhythmCat.Shell\0GetCurrentTrack\0S\0uri\0O\0F\0N\0s\0title\0O\0F\0N\0s\0artist\0O\0F\0N\0s\0album\0O\0F\0N\0s\0comment\0O\0F\0N\0s\0duration\0O\0F\0N\0t\0tracknum\0O\0F\0N\0u\0bitrate\0O\0F\0N\0u\0samplerate\0O\0F\0N\0u\0channel\0O\0F\0N\0u\0\0\0",
"org.supercat.RhythmCat.Shell\0StateChanged\0org.supercat.RhythmCat.Shell\0MusicStarted\0\0",
"\0"
};

