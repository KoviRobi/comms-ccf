/**
\file
\brief FreeRTOS Task which just logs periodically over Comms-CCF.
*/
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/// Use this to start a task which does some logging (logging could come
/// from any task, this is just a demo of it)
void commsCcfStartLogTask();

#ifdef __cplusplus
};
#endif
