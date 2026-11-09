// net.h
#pragma once
void netSetup();
void netLoop();
bool netPublish(const char* topic, const char* payload);