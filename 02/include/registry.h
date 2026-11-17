#ifndef REGISTRY_H
#define REGISTRY_H

#include <sys/types.h>

#include "protocol.h"

typedef struct TopicNode {
    char name[MAX_TOPIC_LENGTH];
    struct TopicNode* next;
} TopicNode;

typedef struct SubscriberNode {
    pid_t pid;
    TopicNode* topics;
    struct SubscriberNode* next;
} SubscriberNode;

typedef struct PublisherNode {
    pid_t pid;
    struct PublisherNode* next;
} PublisherNode;

typedef struct {
    SubscriberNode* subscribers;
    PublisherNode* publishers;
} Registry;

int registry_register_publisher(Registry* registry, pid_t pid);

int registry_add_subscription(Registry* registry, pid_t pid, const char* topic);

void registry_remove_subscription(Registry* registry, pid_t pid, const char* topic);

int subscriber_has_topic(const SubscriberNode* subscriber, const char* topic);

void registry_clear(Registry* registry);

#endif