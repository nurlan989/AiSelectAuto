from ultralytics import YOLO

model = YOLO(
    "models/best.onnx",
    task="detect"
)

results = model(
    "test.jpg",
    conf=0.1,
    iou=0.45,
    imgsz=640
)

for r in results:
    print("Найдено:", len(r.boxes))

    for box in r.boxes:
        cls = int(box.cls[0])
        conf = float(box.conf[0])

        print(
            "Класс:",
            cls,
            "Confidence:",
            conf
        )

    r.save("result.jpg")
